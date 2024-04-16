import java.util.List;
import java.util.ArrayList;
import java.util.random.RandomGenerator;
import java.util.random.RandomGeneratorFactory;

final class Main {

    private static boolean arrayEquals(final byte[] c, final int aStart, final int bStart, final int len) {
        // TODO: vectorizable
        for (int i = 0; i < len; i++) {
            if (c[aStart + i] != c[bStart + i]) {
                return false;
            }
        }
        return true;
    }

    /**
     * For debugging.
     */
    private static void print(final String prefix, final byte[] x) {
        print(prefix, x, 0, 0);
    }

    private static void print(final String prefix, final byte[] x, final int startHighlight, final int endHighlight) {
        System.out.print(prefix);
        int i = 0;
        for (; i < startHighlight; i++) {
            System.out.printf("%02x", x[i]);
        }
        System.out.print("\u001B[33m");
        for (; i < endHighlight; i++) {
            System.out.printf("%02x", x[i]);
        }
        System.out.print("\u001B[0m");
        for (; i < x.length; i++) {
            System.out.printf("%02x", x[i]);
        }
        System.out.println();
    }

    private static byte[] applyTransformation(final byte[] input, final int bestSubstringStart,
            final int bestSubstringLength, final int bestSubstringBytes, final List<Integer> occurrences) {
        final byte[] compressed = new byte[bestSubstringBytes];

        // print("Compressed : ", compressed);
        int i = 0;
        // length of A
        compressed[i++] = (byte) (bestSubstringLength >>> 24);
        compressed[i++] = (byte) ((bestSubstringLength & 0x00ff0000) >>> 16);
        compressed[i++] = (byte) ((bestSubstringLength & 0x0000ff00) >>> 8);
        compressed[i++] = (byte) (bestSubstringLength & 0x000000ff);
        // print("Compressed : ", compressed, 0, i);
        // number of occurrences of A
        compressed[i++] = (byte) (occurrences.size() >>> 24);
        compressed[i++] = (byte) ((occurrences.size() & 0x00ff0000) >>> 16);
        compressed[i++] = (byte) ((occurrences.size() & 0x0000ff00) >>> 8);
        compressed[i++] = (byte) (occurrences.size() & 0x000000ff);
        // print("Compressed : ", compressed, 4, i);

        // position of each occurrence
        for (final int p : occurrences) {
            compressed[i++] = (byte) (p >>> 24);
            compressed[i++] = (byte) ((p & 0x00ff0000) >>> 16);
            compressed[i++] = (byte) ((p & 0x0000ff00) >>> 8);
            compressed[i++] = (byte) (p & 0x000000ff);
            // print("Compressed : ", compressed, i - 4, i);
        }

        // copy A
        System.arraycopy(input, bestSubstringStart, compressed, i, bestSubstringLength);
        i += bestSubstringLength;
        // print("Compressed : ", compressed, i - bestSubstringLength, i);

        // copying portion before A
        if (bestSubstringStart > 0) {
            System.arraycopy(input, 0, compressed, i, bestSubstringStart);
            i += bestSubstringStart;
            // print("Compressed : ", compressed, i - bestSubstringStart, i);
        }

        // copy all the other portions of the array
        int lastPos = bestSubstringStart + bestSubstringLength;
        for (final int p : occurrences) {
            final int portionLength = p - lastPos;
            System.arraycopy(input, lastPos, compressed, i, portionLength);
            lastPos = p + bestSubstringLength;
            i += portionLength;
            // print("Compressed : ", compressed, i - portionLength, i);
        }

        // copy last portion
        final int lastPortionLength = input.length - lastPos;
        System.arraycopy(input, lastPos, compressed, i, lastPortionLength);
        i += lastPortionLength;
        // print("Compressed : ", compressed, i - lastPortionLength, i);

        if (i != bestSubstringBytes) {
            throw new AssertionError(
                    String.format("Expected %,d bytes to be written but we wrote only %,d",
                            bestSubstringBytes, i));
        }

        return compressed;
    }

    private static byte[] compressV1(final byte[] input) {
        System.out.println("Using algorithm v1");
        final int inputLen = input.length;

        int bestSubstringStart = -1;
        int bestSubstringLength = -1;
        List<Integer> positions = null;
        int bestSubstringBytes = Integer.MAX_VALUE;

        for (int substringLength = inputLen / 2; substringLength > 0; substringLength--) {
            // looking for best sequence of length = substringlength
            for (int substringStart = 0; substringStart + substringLength < inputLen; substringStart++) {
                // our substring starts at substringStart (inclusive) and is long
                // substringLength

                // counting occurrences of this substring
                final List<Integer> occurrences = new ArrayList<>();
                // TODO: this can be converted into a search for the first equal byte, and then
                // vectorized
                for (int i = substringStart + substringLength; i + substringLength < inputLen; i++) {
                    // our candidate subtring starts at i (inclusive) and is long substringLength
                    if (arrayEquals(input, substringStart, i, substringLength)) {
                        occurrences.add(i);
                        i += substringLength;
                    }
                }

                if (occurrences.isEmpty()) {
                    continue;
                }

                final int newBytes = 4 // length of A (in bytes)
                        + 4 // number of occurrences of A (in bytes)
                        + 4 * occurrences.size() // position of each of the occurrences (in bytes)
                        + (inputLen - (substringLength * occurrences.size())); // length of the string without all
                                                                               // the occurrences except one

                if (newBytes < bestSubstringBytes) {
                    System.out.printf("Found new best start=%,d; length=%,d; occurrences=%,d; newBytes=%,d\n",
                            substringStart, substringLength, occurrences.size(), newBytes);
                    bestSubstringBytes = newBytes;
                    bestSubstringStart = substringStart;
                    bestSubstringLength = substringLength;
                    positions = occurrences;
                }
            }
        }

        // print("Using : ", input, bestSubstringStart, bestSubstringStart +
        // bestSubstringLength);

        return applyTransformation(input, bestSubstringStart, bestSubstringLength, bestSubstringBytes, positions);
    }

    private static byte[] compressV2(final byte[] input) {
        System.out.println("Using algorithm v2");
        final int inputLen = input.length;

        int bestSubstringStart = -1;
        int bestSubstringLength = -1;
        List<Integer> positions = null;
        int bestSubstringBytes = Integer.MAX_VALUE;

        /*
         * Here we are doing a bruteforce search on all possible substrings. This means
         * that, if we find a substring of length N which is repeated K times in the
         * file, we know for sure that there are N-1 other possible substrings shorter
         * than N which appear at least K times in the file. Similarly, if we can't find
         * a substring of length N repeated at least 2 times, we know for sure that no
         * other substring of length greater than N will b repeated at least 2 times.
         */
        int high = inputLen / 2; // longest possible substring is half the size of the array
        int low = 1; // the shortest possible substring is 1 byte long
        while (low < high) {
            int substringLength = (low + high) / 2;
            System.out.printf("Testing length %,d\n", substringLength);

            // looking for best sequence of length = substringlength
            for (int substringStart = 0; substringStart + substringLength < inputLen; substringStart++) {
                // our substring starts at substringStart (inclusive) and is long
                // substringLength

                // counting occurrences of this substring
                final List<Integer> occurrences = new ArrayList<>();
                // TODO: this can be converted into a search for the first equal byte, and then
                // vectorized
                for (int i = substringStart + substringLength; i + substringLength < inputLen; i++) {
                    // our candidate subtring starts at i (inclusive) and is long substringLength
                    if (arrayEquals(input, substringStart, i, substringLength)) {
                        occurrences.add(i);
                        i += substringLength;
                    }
                }

                if (occurrences.isEmpty()) {
                    // this substring does not repeat
                    high = substringLength - 1;
                    continue;
                }

                final int newBytes = 4 // length of A (in bytes)
                        + 4 // number of occurrences of A (in bytes)
                        + 4 * occurrences.size() // position of each of the occurrences (in bytes)
                        + (inputLen - (substringLength * occurrences.size())); // length of the string without all
                                                                               // the occurrences except one

                if (newBytes < bestSubstringBytes) {
                    System.out.printf("Found new best start=%,d; length=%,d; occurrences=%,d; newBytes=%,d\n",
                            substringStart, substringLength, occurrences.size(), newBytes);
                    bestSubstringBytes = newBytes;
                    bestSubstringStart = substringStart;
                    bestSubstringLength = substringLength;
                    positions = occurrences;
                }

                low = substringLength + 1;
            }
        }

        // print("Using : ", input, bestSubstringStart, bestSubstringStart +
        // bestSubstringLength);

        return applyTransformation(input, bestSubstringStart, bestSubstringLength, bestSubstringBytes, positions);
    }

    private static byte[] randomInput(final int length)  {
        final RandomGenerator rng = RandomGeneratorFactory.getDefault().create(System.nanoTime());
        final byte[] input = new byte[length];
        for (int i = 0; i < input.length; i++) {
            input[i] = (byte) (rng.nextInt() & 0x000000ff);
        }
        return input;
    }

    private static byte[] worstCase() {
        return new byte[] { (byte) 0x00,
                (byte) 0x01,
                (byte) 0x02,
                (byte) 0x03,
                (byte) 0x04,
                (byte) 0x05,
                (byte) 0x06,
                (byte) 0x07,
                (byte) 0x08,
                (byte) 0x09,
                (byte) 0x0a,
                (byte) 0x0b,
                (byte) 0x0c,
                (byte) 0x0d,
                (byte) 0x0e,
                (byte) 0x0f,
                (byte) 0x10,
                (byte) 0x11,
                (byte) 0x12,
                (byte) 0x13,
                (byte) 0x14,
                (byte) 0x15,
                (byte) 0x16,
                (byte) 0x17,
                (byte) 0x18,
                (byte) 0x19,
                (byte) 0x1a,
                (byte) 0x1b,
                (byte) 0x1c,
                (byte) 0x1d,
                (byte) 0x1e,
                (byte) 0x1f,
                (byte) 0x20,
                (byte) 0x21,
                (byte) 0x22,
                (byte) 0x23,
                (byte) 0x24,
                (byte) 0x25,
                (byte) 0x26,
                (byte) 0x27,
                (byte) 0x28,
                (byte) 0x29,
                (byte) 0x2a,
                (byte) 0x2b,
                (byte) 0x2c,
                (byte) 0x2d,
                (byte) 0x2e,
                (byte) 0x2f,
                (byte) 0x30,
                (byte) 0x31,
                (byte) 0x32,
                (byte) 0x33,
                (byte) 0x34,
                (byte) 0x35,
                (byte) 0x36,
                (byte) 0x37,
                (byte) 0x38,
                (byte) 0x39,
                (byte) 0x3a,
                (byte) 0x3b,
                (byte) 0x3c,
                (byte) 0x3d,
                (byte) 0x3e,
                (byte) 0x3f,
                (byte) 0x40,
                (byte) 0x41,
                (byte) 0x42,
                (byte) 0x43,
                (byte) 0x44,
                (byte) 0x45,
                (byte) 0x46,
                (byte) 0x47,
                (byte) 0x48,
                (byte) 0x49,
                (byte) 0x4a,
                (byte) 0x4b,
                (byte) 0x4c,
                (byte) 0x4d,
                (byte) 0x4e,
                (byte) 0x4f,
                (byte) 0x50,
                (byte) 0x51,
                (byte) 0x52,
                (byte) 0x53,
                (byte) 0x54,
                (byte) 0x55,
                (byte) 0x56,
                (byte) 0x57,
                (byte) 0x58,
                (byte) 0x59,
                (byte) 0x5a,
                (byte) 0x5b,
                (byte) 0x5c,
                (byte) 0x5d,
                (byte) 0x5e,
                (byte) 0x5f,
                (byte) 0x60,
                (byte) 0x61,
                (byte) 0x62,
                (byte) 0x63,
                (byte) 0x64,
                (byte) 0x65,
                (byte) 0x66,
                (byte) 0x67,
                (byte) 0x68,
                (byte) 0x69,
                (byte) 0x6a,
                (byte) 0x6b,
                (byte) 0x6c,
                (byte) 0x6d,
                (byte) 0x6e,
                (byte) 0x6f,
                (byte) 0x70,
                (byte) 0x71,
                (byte) 0x72,
                (byte) 0x73,
                (byte) 0x74,
                (byte) 0x75,
                (byte) 0x76,
                (byte) 0x77,
                (byte) 0x78,
                (byte) 0x79,
                (byte) 0x7a,
                (byte) 0x7b,
                (byte) 0x7c,
                (byte) 0x7d,
                (byte) 0x7e,
                (byte) 0x7f,
                (byte) 0x80,
                (byte) 0x81,
                (byte) 0x82,
                (byte) 0x83,
                (byte) 0x84,
                (byte) 0x85,
                (byte) 0x86,
                (byte) 0x87,
                (byte) 0x88,
                (byte) 0x89,
                (byte) 0x8a,
                (byte) 0x8b,
                (byte) 0x8c,
                (byte) 0x8d,
                (byte) 0x8e,
                (byte) 0x8f,
                (byte) 0x90,
                (byte) 0x91,
                (byte) 0x92,
                (byte) 0x93,
                (byte) 0x94,
                (byte) 0x95,
                (byte) 0x96,
                (byte) 0x97,
                (byte) 0x98,
                (byte) 0x99,
                (byte) 0x9a,
                (byte) 0x9b,
                (byte) 0x9c,
                (byte) 0x9d,
                (byte) 0x9e,
                (byte) 0x9f,
                (byte) 0xa0,
                (byte) 0xa1,
                (byte) 0xa2,
                (byte) 0xa3,
                (byte) 0xa4,
                (byte) 0xa5,
                (byte) 0xa6,
                (byte) 0xa7,
                (byte) 0xa8,
                (byte) 0xa9,
                (byte) 0xaa,
                (byte) 0xab,
                (byte) 0xac,
                (byte) 0xad,
                (byte) 0xae,
                (byte) 0xaf,
                (byte) 0xb0,
                (byte) 0xb1,
                (byte) 0xb2,
                (byte) 0xb3,
                (byte) 0xb4,
                (byte) 0xb5,
                (byte) 0xb6,
                (byte) 0xb7,
                (byte) 0xb8,
                (byte) 0xb9,
                (byte) 0xba,
                (byte) 0xbb,
                (byte) 0xbc,
                (byte) 0xbd,
                (byte) 0xbe,
                (byte) 0xbf,
                (byte) 0xc0,
                (byte) 0xc1,
                (byte) 0xc2,
                (byte) 0xc3,
                (byte) 0xc4,
                (byte) 0xc5,
                (byte) 0xc6,
                (byte) 0xc7,
                (byte) 0xc8,
                (byte) 0xc9,
                (byte) 0xca,
                (byte) 0xcb,
                (byte) 0xcc,
                (byte) 0xcd,
                (byte) 0xce,
                (byte) 0xcf,
                (byte) 0xd0,
                (byte) 0xd1,
                (byte) 0xd2,
                (byte) 0xd3,
                (byte) 0xd4,
                (byte) 0xd5,
                (byte) 0xd6,
                (byte) 0xd7,
                (byte) 0xd8,
                (byte) 0xd9,
                (byte) 0xda,
                (byte) 0xdb,
                (byte) 0xdc,
                (byte) 0xdd,
                (byte) 0xde,
                (byte) 0xdf,
                (byte) 0xe0,
                (byte) 0xe1,
                (byte) 0xe2,
                (byte) 0xe3,
                (byte) 0xe4,
                (byte) 0xe5,
                (byte) 0xe6,
                (byte) 0xe7,
                (byte) 0xe8,
                (byte) 0xe9,
                (byte) 0xea,
                (byte) 0xeb,
                (byte) 0xec,
                (byte) 0xed,
                (byte) 0xee,
                (byte) 0xef,
                (byte) 0xf0,
                (byte) 0xf1,
                (byte) 0xf2,
                (byte) 0xf3,
                (byte) 0xf4,
                (byte) 0xf5,
                (byte) 0xf6,
                (byte) 0xf7,
                (byte) 0xf8,
                (byte) 0xf9,
                (byte) 0xfa,
                (byte) 0xfb,
                (byte) 0xfc,
                (byte) 0xfd,
                (byte) 0xfe,

                (byte) 0xff };

    }

    public static void main(final String[] args) {
        byte[] input = randomInput(30_000);
        System.out.printf("Initial length : %,d bytes\n", input.length);

        for (int i = 0; i < 10; i++) {
            System.out.printf("Iteration %,d\n", i);
            // print("Input : ", input);
            input = compressV2(input);
            // print("Output : ", input);
            System.out.printf("New length : %,d bytes\n", input.length);
        }
    }
}