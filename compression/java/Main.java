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

    private static byte[] applyTransformationV1(final byte[] input, final int bestSubstringStart,
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

    private static void writeEncoded(final WriteOnlyBitStream ws, final int x) {
        switch (bitsToEncode(x)) {
            case 8 -> {
                ws.write(true);
                for (int i = 0; i < 7; i++) {
                    ws.write((x & (1 << i)) != 0);
                }
            }
            case 16 -> {
                ws.write(false);
                ws.write(true);
                for (int i = 0; i < 14; i++) {
                    ws.write((x & (1 << i)) != 0);
                }
            }
            case 24 -> {
                ws.write(false);
                ws.write(false);
                ws.write(true);
                for (int i = 0; i < 21; i++) {
                    ws.write((x & (1 << i)) != 0);
                }
            }
            case 32 -> {
                ws.write(false);
                ws.write(false);
                ws.write(false);
                ws.write(true);
                for (int i = 0; i < 28; i++) {
                    ws.write((x & (1 << i)) != 0);
                }
            }
            default -> throw new IllegalArgumentException();
        }
    }

    private static byte[] applyTransformationV2(final byte[] input, final int bestSubstringStart,
            final int bestSubstringLength, final int bestSubstringBits, final List<Integer> occurrences) {
        final ReadOnlyBitStream rs = new ReadOnlyBitStream(input);
        final WriteOnlyBitStream ws = new WriteOnlyBitStream((bestSubstringBits + 7) / 8);

        // length of A
        writeEncoded(ws, bestSubstringLength);

        // number of occurrences of A
        writeEncoded(ws, occurrences.size());

        // position of each occurrence
        for (final int p : occurrences) {
            writeEncoded(ws, p);
        }

        // copy A
        rs.setBitPosition(bestSubstringStart);
        for (int i = 0; i < bestSubstringLength; i++) {
            ws.write(rs.next());
        }

        // copying portion before A
        if (bestSubstringStart > 0) {
            rs.setBitPosition(0);
            for (int i = 0; i < bestSubstringStart; i++) {
                ws.write(rs.next());
            }
        }

        // copy all the other portions of the array
        int lastPos = bestSubstringStart + bestSubstringLength;
        for (final int p : occurrences) {
            rs.setBitPosition(lastPos);
            final int portionLength = p - lastPos;
            for (int i = 0; i < portionLength; i++) {
                ws.write(rs.next());
            }
            lastPos = p + bestSubstringLength;
        }

        // copy last portion
        rs.setBitPosition(lastPos);
        final int lastPortionLength = input.length - lastPos;
        for (int i = 0; i < lastPortionLength; i++) {
            ws.write(rs.next());
        }

        return ws.array();
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

        return applyTransformationV1(input, bestSubstringStart, bestSubstringLength, bestSubstringBytes, positions);
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

        return applyTransformationV1(input, bestSubstringStart, bestSubstringLength, bestSubstringBytes, positions);
    }

    private static final class ReadOnlyBitStream {

        private final byte[] b;
        private int bitIndex = 0;

        public ReadOnlyBitStream(final byte[] b) {
            this.b = b;
        }

        public boolean hasNext() {
            return bitIndex < b.length * 8;
        }

        public boolean next() {
            final int byteIndex = bitIndex >>> 3;
            final int bitInByte = bitIndex & 0x00000007;
            final boolean x = (b[byteIndex] & (1 << bitInByte)) != 0;
            bitIndex++;
            return x;
        }

        public int bitPosition() {
            return bitIndex;
        }

        public void setBitPosition(final int pos) {
            bitIndex = pos;
        }
    }

    private static final class WriteOnlyBitStream {

        private final byte[] b;
        private int bitIndex = 0;

        public WriteOnlyBitStream(final int nBytes) {
            this.b = new byte[nBytes];
        }

        public void write(final boolean v) {
            if (v) {
                final int byteIndex = bitIndex >>> 3;
                final int bitInByte = bitIndex & 0x00000007;
                b[byteIndex] |= (1 << bitInByte);
            }
            bitIndex++;
        }

        public byte[] array() {
            return b;
        }
    }

    private static byte[] compressV3(final byte[] input) {
        System.out.println("Using algorithm v3");
        final int inputBits = input.length * 8;

        final ReadOnlyBitStream bs1 = new ReadOnlyBitStream(input);
        final ReadOnlyBitStream bs2 = new ReadOnlyBitStream(input);

        int bestSubstringStart = -1;
        int bestSubstringLength = -1;
        List<Integer> positions = null;
        int bestSubstringBits = Integer.MAX_VALUE;

        /*
         * Here we are doing a bruteforce search on all possible substrings. This means
         * that, if we find a substring of length N which is repeated K times in the
         * file, we know for sure that there are N-1 other possible substrings shorter
         * than N which appear at least K times in the file. Similarly, if we can't find
         * a substring of length N repeated at least 2 times, we know for sure that no
         * other substring of length greater than N will b repeated at least 2 times.
         */
        int high = inputBits / 2; // longest possible substring is half the size of the array
        int low = 1; // the shortest possible substring is 1 byte long
        while (low < high) {
            int substringLength = (low + high) / 2;
            System.out.printf("Testing length %,d\n", substringLength);

            // looking for best sequence of length = substringlength
            for (int substringStart = 0; substringStart + substringLength < inputBits; substringStart++) {
                // our substring starts at substringStart (inclusive) and is long
                // substringLength

                // counting occurrences of this substring
                final List<Integer> occurrences = new ArrayList<>();
                // TODO: this can be converted into a search for the first equal byte, and then
                // vectorized
                for (int i = substringStart + substringLength; i + substringLength < inputBits; i++) {
                    // our candidate subtring starts at i (inclusive) and is long substringLength
                    bs1.setBitPosition(substringStart);
                    bs2.setBitPosition(i);
                    boolean areEqual = true;
                    for (int k = 0; k < substringLength; k++) {
                        if (bs1.next() != bs2.next()) {
                            areEqual = false;
                            break;
                        }
                    }
                    if (areEqual) {
                        occurrences.add(i);
                        i += substringLength;
                    }
                }

                if (occurrences.isEmpty()) {
                    // this substring does not repeat
                    high = substringLength - 1;
                    continue;
                }

                final int newBits = 32 // length of A (in bits)
                        + 32 // number of occurrences of A (in bits)
                        + 32 * occurrences.size() // position of each of the occurrences (in bits)
                        + (inputBits - (substringLength * occurrences.size())); // length of the string without all
                                                                                // the occurrences except one

                if (newBits < bestSubstringBits) {
                    System.out.printf("Found new best start=%,d; length=%,d; occurrences=%,d; newBits=%,d\n",
                            substringStart, substringLength, occurrences.size(), newBits);
                    bestSubstringBits = newBits;
                    bestSubstringStart = substringStart;
                    bestSubstringLength = substringLength;
                    positions = occurrences;
                }

                low = substringLength + 1;
            }
        }

        // print("Using : ", input, bestSubstringStart, bestSubstringStart +
        // bestSubstringLength);

        return applyTransformationV1(input, bestSubstringStart, bestSubstringLength, bestSubstringBits, positions);
    }

    private static int bitsToEncode(final int x) {
        if (x < 128) {
            return 8;
        }
        if (x < 32768) {
            return 16;
        }
        if (x < 8_388_608) {
            return 24;
        }
        return 32;
    }

    private static byte[] compressV4(final byte[] input) {
        System.out.println("Using algorithm v4");
        final int inputBits = input.length * 8;

        final ReadOnlyBitStream bs1 = new ReadOnlyBitStream(input);
        final ReadOnlyBitStream bs2 = new ReadOnlyBitStream(input);

        int bestSubstringStart = -1;
        int bestSubstringLength = -1;
        List<Integer> bestSubstringOccurrences = new ArrayList<>();
        int bestSubstringBits = Integer.MAX_VALUE;

        /*
         * Here we are doing a bruteforce search on all possible substrings. This means
         * that, if we find a substring of length N which is repeated K times in the
         * file, we know for sure that there are N-1 other possible substrings shorter
         * than N which appear at least K times in the file. Similarly, if we can't find
         * a substring of length N repeated at least 2 times, we know for sure that no
         * other substring of length greater than N will b repeated at least 2 times.
         */
        int high = inputBits / 2; // longest possible substring is half the size of the array
        int low = 1; // the shortest possible substring is 1 byte long
        while (low < high) {
            int substringLength = (low + high) / 2;
            System.out.printf("Testing length %,d\n", substringLength);

            // looking for best sequence of length = substringlength
            for (int substringStart = 0; substringStart
                    + substringLength * Math.max(1, bestSubstringOccurrences.size()) < inputBits; substringStart++) {
                // System.out.printf("Testing start position %,d\n", substringStart);
                // our substring starts at substringStart (inclusive) and is long
                // substringLength

                // counting occurrences of this substring
                final List<Integer> occurrences = new ArrayList<>();
                // TODO: this can be converted into a search for the first equal byte, and then
                // vectorized
                for (int i = substringStart + substringLength; i + substringLength < inputBits; i++) {
                    // our candidate subtring starts at i (inclusive) and is long substringLength
                    bs1.setBitPosition(substringStart);
                    bs2.setBitPosition(i);
                    boolean areEqual = true;
                    for (int k = 0; k < substringLength; k++) {
                        if (bs1.next() != bs2.next()) {
                            areEqual = false;
                            break;
                        }
                    }
                    if (areEqual) {
                        occurrences.add(i);
                        i += substringLength;
                    }
                }

                if (occurrences.isEmpty()) {
                    // this substring does not repeat
                    high = substringLength - 1;
                    continue;
                }

                final int newBits = bitsToEncode(substringLength) // length of A (in bits)
                        + bitsToEncode(occurrences.size()) // number of occurrences of A (in bits)
                        + occurrences.stream().mapToInt(x -> bitsToEncode(x)).sum() // position of each of the
                                                                                    // occurrences (in bits)
                        + (inputBits - (substringLength * occurrences.size())); // length of the string without all
                                                                                // the occurrences except one

                if (newBits < bestSubstringBits) {
                    System.out.printf("Found new best start=%,d; length=%,d; occurrences=%,d; newBits=%,d\n",
                            substringStart, substringLength, occurrences.size(), newBits);
                    bestSubstringBits = newBits;
                    bestSubstringStart = substringStart;
                    bestSubstringLength = substringLength;
                    bestSubstringOccurrences = occurrences;
                }

                low = substringLength + 1;
            }
        }

        // print("Using : ", input, bestSubstringStart, bestSubstringStart +
        // bestSubstringLength);

        return applyTransformationV2(input, bestSubstringStart, bestSubstringLength, bestSubstringBits,
                bestSubstringOccurrences);
    }

    private static byte[] randomInput(final int length) {
        final RandomGenerator rng = RandomGeneratorFactory.getDefault().create(System.nanoTime());
        final byte[] input = new byte[length];
        for (int i = 0; i < input.length; i++) {
            input[i] = (byte) (rng.nextInt() & 0x000000ff);
        }
        return input;
    }

    public static void main(final String[] args) {
        byte[] input = randomInput(10_000);
        System.out.printf("Initial length : %,d bytes (%,d bits)\n", input.length, input.length * 8);

        for (int i = 0; i < 10; i++) {
            System.out.printf("Iteration %,d\n", i);
            // print("Input : ", input);
            input = compressV4(input);
            // print("Output : ", input);
            System.out.printf("New length : %,d bytes\n", input.length);
            System.out.println();
        }
    }
}