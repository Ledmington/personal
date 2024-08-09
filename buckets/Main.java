/*
 * This program is a comparison between different algorithms that solve the same problem.
 * 
 * A problem instance is composed of:
 *  - a capacity C
 *  - a list of weights of objects
 * The goal is to group these objects into buckets, all with the same maximum capacity C, minimizing the "excess" in each bucket.
 * 
 * Constraints:
 *    C >= 1
 *    1 <= w_i <= C for every weight w_i
 *    there is always at least 1 object
 * 
 * Each object can only be "used" once. You cannot use multiple copies of the same object in the same bucket nor in different buckets.
 * You can use as many buckets as you like, at least 1. Remember that empty buckets have a really bad score.
 * 
 * Example:
 *  C = 10
 *  w = [1, 2, 2, 2, 3, 4, 7, 7, 8]
 * A possible solution is [[1, 8], [2, 2, 2, 4], [7, 3], [7]] with a score of 1+0+0+3 = 4.
 * Another (seemingly better) solution would be [[2, 8], [3, 7], [1, 2, 7], [2, 4]] with a score of 0+0+0+4 = 4.
 * 
 * I think this problem is NP-hard because it has a similar structure to many NP-hard problems, but I cannot find it precisely in the literature.
 */

import java.util.Map;
import java.util.HashMap;
import java.util.Objects;
import java.util.List;
import java.util.ArrayList;
import java.util.random.RandomGenerator;
import java.util.random.RandomGeneratorFactory;

final class Main {

    private static final RandomGenerator rng = RandomGeneratorFactory.getDefault().create(System.nanoTime());

    private static record Instance(int capacity, List<Integer> weights) {
    }

    private static Instance generateInstance() {
        final int capacity = rng.nextInt(10, 1_000);
        final int nObjects = rng.nextInt(1, 100);
        final List<Integer> weights = new ArrayList<>(nObjects);
        for (int i = 0; i < nObjects; i++) {
            weights.add(rng.nextInt(1, capacity));
        }
        return new Instance(capacity, weights);
    }

    private static void checkValidInstance(final Instance x) {
        Objects.requireNonNull(x);
        if (x.capacity() <= 0) {
            throw new IllegalArgumentException(
                    String.format("Invalid capacity: expected >0 but was %,d", x.capacity()));
        }
        Objects.requireNonNull(x.weights());
        for (final int w : x.weights()) {
            if (w <= 0 || w >= x.capacity()) {
                throw new IllegalArgumentException(
                        String.format("Invalid weight: expected to be >0 and <=%,d but was %,d", x.capacity(), w));
            }
        }
    }

    private static void checkValidSolution(final Instance x, final List<List<Integer>> s) {
        Objects.requireNonNull(s);

        {
            final Map<Integer, Integer> instanceCounts = new HashMap<>();
            for (final Integer w : x.weights()) {
                if (!instanceCounts.containsKey(w)) {
                    instanceCounts.put(w, 1);
                } else {
                    instanceCounts.put(w, instanceCounts.get(w) + 1);
                }
            }

            final Map<Integer, Integer> counts = new HashMap<>();
            for (int i = 0; i < s.size(); i++) {
                final List<Integer> b = s.get(i);
                for (final int n : b) {
                    if (n <= 0 || n > x.capacity()) {
                        throw new IllegalArgumentException(
                                String.format("Invalid element in bucket %,d: expected to be >0 and <= %,d but was %,d",
                                        i, x.capacity(), n));
                    }
                    if (!x.weights().contains(n)) {
                        throw new IllegalArgumentException(
                                String.format("Element %,d in bucket %,d does not exist", n, i));
                    }
                    if (!counts.containsKey(n)) {
                        counts.put(n, 1);
                    } else {
                        counts.put(n, counts.get(n) + 1);
                    }
                }
            }

            for (final Map.Entry<Integer, Integer> e : instanceCounts.entrySet()) {
                if (!counts.containsKey(e.getKey())) {
                    throw new IllegalArgumentException(String.format(
                            "The weight %,d appeared in the problem instance but was not present in the solution",
                            e.getKey()));
                }
                if (counts.get(e.getKey()) != e.getValue()) {
                    throw new IllegalArgumentException(String.format(
                            "Weight %,d appeared %,d times in the problem instance and %,d times in the solution",
                            e.getKey(), e.getValue(), counts.get(e.getKey())));
                }
            }
            if (!instanceCounts.equals(counts)) {
                throw new Error("Something went wrong");
            }
        }

        {
            for (int i = 0; i < s.size(); i++) {
                final List<Integer> b = s.get(i);
                final int sum = b.stream().mapToInt(a -> a).sum();
                if (sum <= 0 || sum > x.capacity()) {
                    throw new IllegalArgumentException(
                            String.format("Invalid total size of bucket %,d: expected >0 and <= %,d but was %,d", i,
                                    x.capacity(), sum));
                }
            }
        }
    }

    private static int evaluateSolution(final Instance x, final List<List<Integer>> s) {
        int padding = 0;
        for (final List<Integer> b : s) {
            final int sum = b.stream().mapToInt(a -> a).sum();
            padding += (x.capacity() - sum);
        }
        return padding;
    }

    private static List<List<Integer>> greedy(final Instance x) {
        final List<List<Integer>> s = new ArrayList<>();

        int i = 0;
        while (i < x.weights().size()) {
            final List<Integer> bucket = new ArrayList<>();
            int bucketSize = 0;
            while (i < x.weights().size() && bucketSize + x.weights().get(i) <= x.capacity()) {
                bucket.add(x.weights().get(i));
                bucketSize += x.weights().get(i);
                i++;
            }

            s.add(bucket);
        }

        return s;
    }

    private static List<List<Integer>> sortingPlusGreedy(final Instance x) {
        final List<Integer> sortedWeights = new ArrayList<>(x.weights());
        sortedWeights.sort((a, b) -> a - b);
        return greedy(new Instance(x.capacity(), sortedWeights));
    }

    private static List<List<Integer>> createPairs(final Instance x) {
        final List<Integer> sortedWeights = new ArrayList<>(x.weights());
        sortedWeights.sort((a, b) -> a - b);
        final List<List<Integer>> buckets = new ArrayList<>();

        while (!sortedWeights.isEmpty()) {
            // take the last element
            final int i = sortedWeights.size() - 1;
            int j = -1;
            while (j < i && sortedWeights.get(i) + sortedWeights.get(j + 1) <= x.capacity()) {
                j++;
            }

            if (j < 1) {
                // we have not found a pair
                buckets.add(List.of(sortedWeights.get(i)));
                sortedWeights.remove(i);
            } else {
                // we have found a pair
                buckets.add(List.of(sortedWeights.get(i), sortedWeights.get(j - 1)));
                sortedWeights.remove(i);
                sortedWeights.remove(j - 1);
            }
        }

        return buckets;
    }

    private static List<List<Integer>> buildGroups(final Instance x) {
        final List<Integer> sortedWeights = new ArrayList<>(x.weights());
        sortedWeights.sort((a, b) -> a - b);
        final List<List<Integer>> buckets = new ArrayList<>();

        while (!sortedWeights.isEmpty()) {
            final List<Integer> currentBucket = new ArrayList<>();
            int currentBucketSize = 0;

            // we take always the last element
            currentBucket.add(sortedWeights.getLast());
            currentBucketSize += sortedWeights.getLast();
            sortedWeights.remove(sortedWeights.size() - 1);

            while (!sortedWeights.isEmpty()) {
                int i = -1;
                while (i < sortedWeights.size() - 1 && currentBucketSize + sortedWeights.get(i + 1) <= x.capacity()) {
                    i++;
                }

                if (i < 1) {
                    // we have not found another element to insert
                    break;
                }

                currentBucketSize += sortedWeights.get(i - 1);
                currentBucket.add(sortedWeights.get(i - 1));
                sortedWeights.remove(i - 1);
            }

            buckets.add(currentBucket);
        }

        return buckets;
    }

    public static void main(final String[] args) {
        final Instance x = generateInstance();
        System.out.printf("Capacity : %,d\n", x.capacity());
        System.out.printf("Weights  : %s\n", x.weights());
        System.out.printf("Sorted weights : %s\n", x.weights().stream().sorted().toList());
        checkValidInstance(x);

        final List<List<Integer>> greedySolution = greedy(x);
        System.out.println("\n Greedy");
        System.out.printf("Solution : %s\n", greedySolution);
        checkValidSolution(x, greedySolution);
        System.out.printf("Padding  : %,d\n", evaluateSolution(x, greedySolution));

        final List<List<Integer>> sortingPlusGreedySolution = sortingPlusGreedy(x);
        System.out.println("\n Sorting + Greedy");
        System.out.printf("Solution : %s\n", sortingPlusGreedySolution);
        checkValidSolution(x, sortingPlusGreedySolution);
        System.out.printf("Padding  : %,d\n", evaluateSolution(x, sortingPlusGreedySolution));

        final List<List<Integer>> createPairsSolution = createPairs(x);
        System.out.println("\n Create Pairs");
        System.out.printf("Solution : %s\n", createPairsSolution);
        checkValidSolution(x, createPairsSolution);
        System.out.printf("Padding  : %,d\n", evaluateSolution(x, createPairsSolution));

        final List<List<Integer>> buildGroupsSolution = buildGroups(x);
        System.out.println("\n Build Groups");
        System.out.printf("Solution : %s\n", buildGroupsSolution);
        checkValidSolution(x, buildGroupsSolution);
        System.out.printf("Padding  : %,d\n", evaluateSolution(x, buildGroupsSolution));
    }
}