package mulper;

public final class MulPer {

    // Computes the multiplicative persistence of the given number
    private static long mulper(final long n) {
        if (n < 0) {
            throw new IllegalArgumentException("Negative numbers are not allowed.");
        }
        if (n < 10) {
            return 0;
        }

        long result = 1;
        long x = n;
        while (x > 1) {
            result *= (x % 10);
            x /= 10;
        }

        return 1 + mulper(result);
    }

    public static void main(final String[] args) {
        long current = 0;
        for (long n = 0; current <= 11; n++) {
            if (mulper(n) == current) {
                System.out.printf("Smallest n with mul.per. == %d: %,d\n", current, n);
                current++;
            }
        }
    }
}