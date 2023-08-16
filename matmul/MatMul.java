import java.util.random.RandomGenerator;
import java.util.random.RandomGeneratorFactory;

import jdk.incubator.vector.DoubleVector;
import jdk.incubator.vector.VectorSpecies;

public final class MatMul {

    private static final RandomGenerator rng = RandomGeneratorFactory.getDefault().create(System.nanoTime());
    private static final VectorSpecies<Double> species = DoubleVector.SPECIES_PREFERRED;

    private static void naive_matmul_2d(final double[][] a, final double[][] b, double[][] c, final int size) {
        for (int i = 0; i < size; i++) {
            for (int k = 0; k < size; k++) {
                final double aik = a[i][k];
                for (int j = 0; j < size; j++) {
                    c[i][j] += aik * b[k][j];
                }
            }
        }
    }

    private static void naive_matmul_1d(final double[] a, final double[] b, double[] c, final int size) {
        for (int i = 0; i < size; i++) {
            for (int k = 0; k < size; k++) {
                final double aik = a[i * size + k];
                for (int j = 0; j < size; j++) {
                    c[i * size + j] += aik * b[k * size + j];
                }
            }
        }
    }

    private static void vector_matmul_2d(final double[][] a, final double[][] b, double[][] c, final int size) {
        final int alignedLength = species.loopBound(size);

        for (int i = 0; i < size; i++) {
            for (int k = 0; k < size; k++) {
                final double aik = a[i][k];
                final DoubleVector vaik = DoubleVector.broadcast(species, aik);
                int j = 0;
                for (; j < alignedLength; j += species.length()) {
                    final DoubleVector bkj = DoubleVector.fromArray(species, b[k], j);
                    DoubleVector cij = DoubleVector.fromArray(species, c[i], j);
                    cij = vaik.fma(bkj, cij);
                    cij.intoArray(c[i], j);
                }

                // remainder loop
                for (; j < size; j++) {
                    c[i][j] += aik * b[k][j];
                }
            }
        }
    }

    private static void vector_matmul_1d(final double[] a, final double[] b, double[] c, final int size) {
        final int alignedLength = species.loopBound(size);

        for (int i = 0; i < size; i++) {
            for (int k = 0; k < size; k++) {
                final double aik = a[i * size + k];
                final DoubleVector vaik = DoubleVector.broadcast(species, aik);
                int j = 0;
                for (; j < alignedLength; j += species.length()) {
                    final DoubleVector bkj = DoubleVector.fromArray(species, b, k * size + j);
                    DoubleVector cij = DoubleVector.fromArray(species, c, i * size + j);
                    cij = vaik.fma(bkj, cij);
                    cij.intoArray(c, i * size + j);
                }

                // remainder loop
                for (; j < size; j++) {
                    c[i * size + j] += aik * b[k * size + j];
                }
            }
        }
    }

    private static long getTimerResolution() {
        final long[] v = new long[1_000_000];
        for (int i = 0; i < v.length; i++) {
            v[i] = System.nanoTime();
        }
        long res = v[1] - v[0];
        for (int i = 1; i < v.length - 1; i++) {
            res = Math.min(res, v[i + 1] - v[i]);
        }
        return res;
    }

    private static void init_2d(final double[][] a, final double[][] b, final double[][] c, final int size) {
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                a[i][j] = rng.nextDouble(0.0, 1.0);
                b[i][j] = rng.nextDouble(0.0, 1.0);
                c[i][j] = 0.0;
            }
        }
    }

    private static void init_1d(final double[] a, final double[] b, final double[] c, final int size) {
        for (int i = 0; i < size * size; i++) {
            a[i] = rng.nextDouble(0.0, 1.0);
            b[i] = rng.nextDouble(0.0, 1.0);
            c[i] = 0.0;
        }
    }

    private static void check_2d(final double[][] c, final int size) {
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (c[i][j] < 0 || c[i][j] > (double) size) {
                    System.out.printf("ERROR: invalid result at index %d; %d\n", i, j);
                }
            }
        }
    }

    private static void check_1d(final double[] c, final int size) {
        for (int i = 0; i < size * size; i++) {
            if (c[i] < 0 || c[i] > (double) size) {
                System.out.printf("ERROR: invalid result at index %d; %d\n", i);
            }
        }
    }

    private static double benchmark(final Runnable initialization, final Runnable task, final Runnable check) {
        initialization.run();

        final long start = System.nanoTime();
        task.run();
        final long end = System.nanoTime();

        check.run();

        return (double) (end - start) / 1_000_000_000.0;
    }

    private static void runBenchmarks(final int size, final int maxIterations) {
        final double[][] a_2d = new double[size][size];
        final double[][] b_2d = new double[size][size];
        final double[][] c_2d = new double[size][size];
        final double[] a_1d = new double[size * size];
        final double[] b_1d = new double[size * size];
        final double[] c_1d = new double[size * size];

        System.out.println("     | Naive 2D | Vector 2D | Naive 1D | Vector 1D |");

        for (int it = 0; it < maxIterations; it++) {
            System.out.printf("| %2d |", it);
            System.out.printf(" %8.6f |", benchmark(() -> init_2d(a_2d, b_2d, c_2d, size),
                    () -> naive_matmul_2d(a_2d, b_2d, c_2d, size), () -> check_2d(c_2d, size)));
            System.out.printf(" %8.6f |", benchmark(() -> init_2d(a_2d, b_2d, c_2d, size),
                    () -> vector_matmul_2d(a_2d, b_2d, c_2d, size), () -> check_2d(c_2d, size)));

            System.out.printf(" %8.6f |", benchmark(() -> init_1d(a_1d, b_1d, c_1d, size),
                    () -> naive_matmul_1d(a_1d, b_1d, c_1d, size), () -> check_1d(c_1d, size)));
            System.out.printf(" %8.6f |", benchmark(() -> init_1d(a_1d, b_1d, c_1d, size),
                    () -> vector_matmul_1d(a_1d, b_1d, c_1d, size), () -> check_1d(c_1d, size)));

            System.out.println();
        }
    }

    public static void main(final String[] args) {
        int maxIterations = 20;
        int size = 2000;

        // input parsing
        if (args.length > 0) {
            maxIterations = Integer.parseInt(args[0]);
        }
        if (args.length > 1) {
            size = Integer.parseInt(args[1]);
        }
        if (args.length > 2) {
            System.out.println("WARNING: more than 2 arguments provided, ignoring the others.");
        }

        // input checking
        if (maxIterations < 1) {
            System.err.printf("%d is not a valid number of iterations.\n", maxIterations);
            return;
        }
        if (size < 1) {
            System.err.printf("%d is not a valid matrix size.\n", size);
            return;
        }

        System.out.printf("Matrix size               : %,d\n", size);
        System.out.printf("Iterations                : %,d\n", maxIterations);
        System.out.printf("Timer resolution          : %,d ns\n", getTimerResolution());
        System.out.printf("Approx. total memory used : %.3f MB\n",
                (double) (6 * (long) size * (long) size * 8) / 1048576.0);
        System.out.printf("Vector species            : %s\n", species.toString());
        System.out.println();

        runBenchmarks(size, maxIterations);
    }
}