import java.math.BigInteger;
import java.util.Locale;

public final class Main {

    private static BigInteger convertToBigInteger(final String x) {
        if (!x.chars().allMatch(c -> Character.isAlphabetic(c))) {
            throw new IllegalArgumentException(String.format("Invalid string: '%s'.", x));
        }
        final String s = x.toLowerCase(Locale.US);
        if (s.isEmpty()) {
            return BigInteger.ZERO;
        }
        BigInteger n = BigInteger.ONE;
        final BigInteger b = BigInteger.valueOf(26);
        BigInteger e = BigInteger.ONE;
        for (int i = x.length() - 1; i >= 0; i--) {
            n = n.add(BigInteger.valueOf(s.charAt(i) - 'a').multiply(e));
            e = e.multiply(b);
        }
        return n;
    }

    private static String convertToString(BigInteger x) {
        if (x.compareTo(BigInteger.ZERO) < 0) {
            throw new IllegalArgumentException("Negative number.");
        }
        if (x.compareTo(BigInteger.ZERO) == 0) {
            return "";
        }
        if (x.compareTo(BigInteger.ONE) == 0) {
            return "a";
        }
        x = x.subtract(BigInteger.ONE);
        final BigInteger b = BigInteger.valueOf(26);
        final StringBuilder s = new StringBuilder();
        while (x.compareTo(BigInteger.ZERO) > 0) {
            final BigInteger r = x.mod(b);
            s.insert(0, (char) ('a' + r.intValue()));
            x = x.divide(b);
        }
        return s.toString();
    }

    public static void main(final String[] args) {
        final String a = args[0];
        final String b = args[1];

        final BigInteger x = convertToBigInteger(a);
        final BigInteger y = convertToBigInteger(b);

        System.out.printf("%s -> %,d%n", a, x);
        System.out.printf("%s -> %,d%n", b, y);

        System.out.printf("%s x %s = %s%n", a, b, convertToString(x.multiply(y)));
    }
}