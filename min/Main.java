import java.util.List;
import java.util.Objects;
import java.util.ArrayList;
import java.util.random.RandomGenerator;
import java.util.random.RandomGeneratorFactory;

final class Main {

    private static final RandomGenerator rng = RandomGeneratorFactory.getDefault().create(System.nanoTime());

    private static record Point(double[] x, double f) {
        @Override
        public String toString() {
            final StringBuilder sb = new StringBuilder();
            sb.append("f (");
            for (int i = 0; i < x.length; i++) {
                sb.append(String.format("%+.6f", x[i]));
                if (i < x.length - 1) {
                    sb.append(", ");
                }
            }
            sb.append(") = ").append(String.format("%+.6f", f));
            return sb.toString();
        }
    }

    private static double distance(final Point a, final Point b) {
        Objects.requireNonNull(a);
        Objects.requireNonNull(b);
        double s = 0.0;
        for (int i = 0; i < a.x().length; i++) {
            final double diff = a.x()[i] - b.x()[i];
            s += diff * diff;
        }
        return Math.sqrt(s);
    }

    private static double objectiveFunction(final double[] x) {
        Objects.requireNonNull(x);
        if (x.length == 0) {
            throw new IllegalArgumentException("Empty array.");
        }

        double s = 0.0;
        for (int i = 0; i < x.length; i++) {
            s += x[i] * x[i];
        }
        return Math.sqrt(s);
    }

    private static double[] generateRandomNeighbor(final List<Point> points) {
        Objects.requireNonNull(points);
        if (points.isEmpty()) {
            throw new IllegalArgumentException("Empty list.");
        }

        // for (final Point p : points) {
        // System.out.println(p);
        // }

        final double[] x = new double[points.getFirst().x().length];

        final double[] weights = new double[points.size()];
        double minWeight = Double.MAX_VALUE;
        double maxWeight = -Double.MAX_VALUE;
        for (int i = 0; i < points.size(); i++) {
            // since this is a minimization problem, we want points with lower 'f' value to
            // have the highest weight
            weights[i] = -points.get(i).f();

            minWeight = Math.min(minWeight, weights[i]);
            maxWeight = Math.max(maxWeight, weights[i]);
        }

        // Normalize the weights from [min_value; max_value] to [0; 1].
        // Note: this will put the worst value with a weight of 0 and the best value
        // with a weight of 1
        double sumWeights = 0.0;
        for (int i = 0; i < weights.length; i++) {
            weights[i] = (weights[i] - minWeight) / (maxWeight - minWeight);
            sumWeights += weights[i];
            System.out.printf("%s (%.6f)\n", points.get(i), weights[i]);
        }

        // Choosing a normal distribution based on the computed weights
        final double choice = rng.nextDouble(0.0, sumWeights);
        int idx = 0;
        while (idx < weights.length - 1 && choice > weights[idx + 1]) {
            idx++;
        }

        // Find closest point
        int closestPointIndex = -1;
        double closestDistance = Double.MAX_VALUE;
        for (int i = 0; i < points.size(); i++) {
            if (i == idx) {
                continue;
            }
            final double d = distance(points.get(idx), points.get(i));
            if (d < closestDistance) {
                closestDistance = d;
                closestPointIndex = i;
            }
        }

        // Choose sigma as half of the closest distance (good when the two points have
        // the same function value)
        // final double sigma = closestDistance / 2.0;

        // f1 * sigma1 + f2 * sigma2 = closestDistance
        // f1 : f2 = sigma1 : sigma2 -> sigma2 = (f2 * sigma1) / f1
        // f1 * sigma1 + f2 * (f2 * sigma1) / f1 = closestDistance
        // solve for sigma1
        // sigma1 * (f1 + ((f2^2)/f1)) - closestDistance = 0
        // sigma1 = closestDistance / (f1 + ((f2^2)/f1))
        final double f1 = points.get(idx).f();
        final double f2 = points.get(closestPointIndex).f();
        final double sigma = closestDistance / (f1 + (f2 * f2) / f1);

        for (int i = 0; i < x.length; i++) {
            x[i] = rng.nextGaussian(points.get(idx).x()[i], sigma);
        }

        return x;
    }

    public static void main(final String[] args) {
        // The number of dimensions we are considering
        final int dims = 6;

        // Maximum number of iterations
        final int maxIterations = 10;

        final List<Point> points = new ArrayList<>();

        // Generating first 2 points
        {
            final double[] x = new double[dims];
            for (int i = 0; i < dims; i++) {
                x[i] = rng.nextDouble(-5.0, 5.0);
            }
            final double f = objectiveFunction(x);
            points.add(new Point(x, f));
        }
        {
            final double[] x = new double[dims];
            for (int i = 0; i < dims; i++) {
                x[i] = rng.nextDouble(-5.0, 5.0);
            }
            final double f = objectiveFunction(x);
            points.add(new Point(x, f));
        }

        int currentIteration = 0;
        while (currentIteration < maxIterations) {
            System.out.printf("Iteration %,d\n", currentIteration);
            final double[] next = generateRandomNeighbor(points);
            final double f = objectiveFunction(next);
            points.add(new Point(next, f));

            System.out.println(points.getLast());
            System.out.println();

            currentIteration++;
        }

        System.out.println();

        // Find the best point among the visited ones
        int idx = 0;
        for (int i = 0; i < points.size(); i++) {
            if (points.get(i).f() < points.get(idx).f()) {
                idx = i;
            }
        }

        System.out.println("Approx. minimum");
        System.out.println(points.get(idx));
    }
}