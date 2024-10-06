import java.math.BigInteger;
import java.util.Set;
import java.util.HashSet;
import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.stream.Collectors;

/**
 * Inspired by this Numberphile video:
 * https://www.youtube.com/watch?v=OtYKDzXwDEE
 * 
 * More info about the Aliquot sequence here:
 * https://en.wikipedia.org/wiki/Aliquot_sequence
 */
public final class Aliquot {

    private static enum Status {
        ONE_REACHED,
        PERFECT_NUMBER_REACHED,
        LOOP_REACHED,
        DIVERGED
    }

    private static record Result(Status status, List<BigInteger> sequence) {
    }

    private static Map<BigInteger, BigInteger> cache = new HashMap<>();

    private static BigInteger sqrt(final BigInteger x) {
        if (x.bitLength() < 1) {
            return BigInteger.ZERO;
        }
        BigInteger div = BigInteger.ZERO.setBit(x.bitLength() / 2);
        BigInteger div2 = div;
        while (true) {
            final BigInteger y = div.add(x.divide(div)).shiftRight(1);
            if (y.equals(div) || y.equals(div2)) {
                return y;
            }
            div2 = div;
            div = y;
        }
    }

    private static BigInteger aliquotNext(final BigInteger x) {
        if (cache.containsKey(x)) {
            return cache.get(x);
        }
        BigInteger s = BigInteger.ZERO;
        final BigInteger n = sqrt(x);
        for (BigInteger i = BigInteger.ONE; i.compareTo(n) < 0; i = i.add(BigInteger.ONE)) {
            if (x.mod(i).equals(BigInteger.ZERO)) {
                s = s.add(i);
                final BigInteger other = x.divide(i);
                if (!other.equals(i) && !other.equals(x) && other.multiply(i).equals(x)) {
                    s = s.add(other);
                }
            }
        }
        cache.put(x, s);
        return s;
    }

    private static Result aliquotSequence(BigInteger x) {
        if (x.compareTo(BigInteger.valueOf(4)) < 0) {
            return new Result(Status.ONE_REACHED, List.of(x, BigInteger.ONE));
        }
        final int maxSteps = 80;
        final Set<BigInteger> visited = new HashSet<>();
        final List<BigInteger> sequence = new ArrayList<>();
        for (int it = 0; it < maxSteps; it++) {
            final BigInteger next = aliquotNext(x);
            if (next.equals(BigInteger.ONE)) {
                sequence.add(next);
                return new Result(Status.ONE_REACHED, sequence);
            } else if (next.equals(x)) {
                sequence.add(next);
                return new Result(Status.PERFECT_NUMBER_REACHED, sequence);
            } else if (visited.contains(next)) {
                sequence.add(next);
                return new Result(Status.LOOP_REACHED, sequence);
            } else {
                visited.add(x);
                sequence.add(x);
                x = next;
            }
        }
        return new Result(Status.DIVERGED, sequence);
    }

    public static void main(final String[] args) {
        System.out.println(sqrt(BigInteger.valueOf(2)));
        for (int i = 2; i < 1000; i++) {
            final BigInteger x = BigInteger.valueOf(i);
            final long start = System.nanoTime();
            final Result res = aliquotSequence(x);
            final long end = System.nanoTime();
            System.out.printf(" %,6d has reached %,d after %,d steps (%s) in %.3f s\nSequence: %s\n\n", i,
                    res.sequence().getLast(),
                    res.sequence().size(),
                    res.status(), (double) (end - start) / 1_000_000_000.0,
                    res.sequence().stream().map(BigInteger::toString).collect(Collectors.joining(" -> ")));
        }
    }
}