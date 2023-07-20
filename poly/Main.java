import java.util.List;
import java.util.ArrayList;
import java.util.Random;

final class Main {

    private static final Random rnd = new Random(System.nanoTime());

    private static final class Complex {
        private double re;
        private double im;
        public Complex(final double r, final double i) {
            this.re = r;
            this.im = i;
        }
        public Complex() {
            this(0, 0);
        }
        public Complex add(final Complex c) {
            return new Complex(this.re+c.re, this.im+c.im);
        }
        public Complex subtract(final Complex c) {
            return new Complex(this.re-c.re, this.im-c.im);
        }
        public Complex multiply(final Complex c) {
            return new Complex(this.re*c.re - this.im*c.im, this.re*c.im + this.im*c.re);
        }
        public Complex divide(final Complex c) {
            return new Complex((this.re*c.re+this.im*c.im)/(c.re*c.re + c.im*c.im), (this.im*c.re - this.re*c.im)/(c.re*c.re + c.im*c.im));
        }
        public double mod() {
            return Math.hypot(re, im);
        }
        public String toString() {
            return String.format("%+f%+fi", re, im);
        }
    }

    private static Complex compute(final double[] p, final Complex x) {
        Complex result = new Complex(0,0);
        Complex v = new Complex(1,0);
        for(int i=p.length-1; i>=0; i--) {
            result = result.add(v.multiply(new Complex(p[i], 0)));
            v = v.multiply(x);
        }
        return result;
    }

    private static Complex derivative(final double[] p, final Complex x) {
        final double h = 1e-6;
        // we compute the derivative by using the definition (f(x+h)-f(x)) / h, for a "small enough" h
        return compute(p,x.add(new Complex(h,0))).subtract(compute(p,x)).divide(new Complex(h, 0));
    }

    private static List<Complex> findRoots(final double[] p) {
        final List<Complex> sol = new ArrayList<>();
        final double eps = 1e-6;
        final int attempts = 1000;
        final int maxIterations = 1000;

        for(int i=0; i<attempts; i++) {
            Complex current;
            Complex next = new Complex(rnd.nextDouble() * 2_000_000 -1_000_000, rnd.nextDouble() * 2_000_000 -1_000_000);
            int it = 0;
            do {
                current = next;
                // actual computation of Newton's method
                next = current.subtract(compute(p, current).divide(derivative(p, current)));
                it++;
            } while(it < maxIterations && next.subtract(current).mod() > eps);
           
            if(compute(p, current).mod() > eps) {
                // if the found solution is too far from 0, it means that we ran out of iterations
                continue;
            }
           
            boolean alreadyFound = false;
            for(final Complex d : sol) {
                if(d.subtract(current).mod() <= eps) {
                    alreadyFound = true;
                    break;
                }
            }
            if(!alreadyFound) {
                sol.add(current);
            }
        }

        return sol;
    }

    public static void main(final String[] args) {
        // the coefficients are real numbers
        final double[] p = new double[] {1,0,1,0,0,-2,-3,4};
       
        System.out.print("The polynomial is:\np(x) = ");
        for(int i=0; i<p.length-1; i++) {
            if(p[i] == 0) {continue;}
            System.out.printf("%+f*x^%d ", p[i], p.length-i-1);
        }
        System.out.printf("%+f\n\n", p[p.length-1]);


        final List<Complex> solutions = findRoots(p);
       
        System.out.printf("Found %d roots\n", solutions.size());

        for(final Complex d : solutions) {
            final Complex y = compute(p, d);
            System.out.printf("p( %s ) = %s (%f)\n", d, y, y.mod());
        }
    }
}