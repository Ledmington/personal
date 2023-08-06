import java.util.random.RandomGenerator;
import java.util.random.RandomGeneratorFactory;

import jdk.incubator.vector.DoubleVector;
import jdk.incubator.vector.Vector;
import jdk.incubator.vector.VectorSpecies;

public final class MatMul {

    private static void matmul(final double[][] a, final double[][] b, double[][] c, final int size) {
        for (int i = 0; i < size; i++) {
            for (int k = 0; k < size; k++) {
                for (int j = 0; j < size; j++) {
                    c[i][j] += a[i][k] * b[k][j];
                }
            }
        }
    }

    private static void vector_matmul(final double[][] a, final double[][] b, double[][] c, final int size) {
        final VectorSpecies<Double> species = DoubleVector.SPECIES_PREFERRED;
        final int alignedLength = species.loopBound(size);

        for (int i = 0; i < size; i++) {
            for (int k = 0; k < size; k++) {
                final DoubleVector a_i_k = DoubleVector.broadcast(species, a[i][k]);
                for (int j = 0; j < alignedLength; j += species.length()) {
                    final DoubleVector b_k_j = DoubleVector.fromArray(species, b[k], j);
                    DoubleVector c_i_j = DoubleVector.fromArray(species, c[i], j);
                    c_i_j = a_i_k.fma(b_k_j, c_i_j);
                    c_i_j.intoArray(c[i], j);
                }
            }
        }
    }

    public static void main(final String[] args) {
        final RandomGenerator rng = RandomGeneratorFactory.getDefault().create(System.nanoTime());

        final int size = 1024;

        final double[][] a = new double[size][size];
        final double[][] b = new double[size][size];
        final double[][] c = new double[size][size];

        final int max_iterations = 100;
        long start;
        long end;

        for (int it = 0; it < max_iterations; it++) {
            // matrix initialization
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                    a[i][j] = rng.nextDouble(0.0, 1.0);
                    b[i][j] = rng.nextDouble(0.0, 1.0);
                    c[i][j] = 0.0;
                }
            }

            start = System.nanoTime();
            matmul(a, b, c, size);
            end = System.nanoTime();

            // check result
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                    if (c[i][j] < 0 || c[i][j] > (double) size) {
                        System.out.printf("ERROR: invalid result at index %d; %d\n", i, j);
                    }
                }
            }

            System.out.printf("%7.6f seconds", (double) (end - start) / 1_000_000_000.0);

            // matrix initialization
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                    a[i][j] = rng.nextDouble(0.0, 1.0);
                    b[i][j] = rng.nextDouble(0.0, 1.0);
                    c[i][j] = 0.0;
                }
            }

            start = System.nanoTime();
            vector_matmul(a, b, c, size);
            end = System.nanoTime();

            // check result
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                    if (c[i][j] < 0 || c[i][j] > (double) size) {
                        System.out.printf("ERROR: invalid result at index %d; %d\n", i, j);
                    }
                }
            }

            System.out.printf("\t%7.6f seconds\n", (double) (end - start) / 1_000_000_000.0);
        }
    }
}