package diet;

import java.io.IOException;

public final class Main {

    private static void printTableau(final double[][] tab) {
        for (int i = 0; i < tab.length; i++) {
            for (int j = 0; j < tab[i].length; j++) {
                System.out.printf("%+3.3f ", tab[i][j]);
            }
            System.out.println();
        }
    }

    private static boolean isOptimalSolution(final double[][] tab) {
        for (int i = 0; i < tab[0].length; i++) {
            if (tab[tab.length - 1][i] < 0) {
                return false;
            }
        }
        return true;
    }

    private static int findBestEnteringVariable(final double[][] tab) {
        int bestIdx = 0;
        for (int i = 1; i < tab[tab.length - 1].length - 2; i++) {
            if (tab[tab.length - 1][i] < tab[tab.length - 1][bestIdx]) {
                bestIdx = i;
            }
        }
        return bestIdx;
    }

    private static int findWithBlandRule(final double[][] tab) {
        for (int i = 0; i < tab[tab.length - 1].length - 2; i++) {
            if (tab[tab.length - 1][i] < 0) {
                return i;
            }
        }
        return -1;
    }

    private static boolean checkFiniteSolutionExists(final double[][] tableau, final int enteringVariable) {
        for (int i = 0; i < tableau.length - 1; i++) {
            if (tableau[i][enteringVariable] >= 0) {
                return true;
            }
        }
        return false;
    }

    private static int findMinRow(final double[][] tab, final int enteringVariable) {
        int bestIdx = 0;
        double bestValue = Double.MAX_VALUE;
        for (int i = 0; i < tab.length - 1; i++) {
            final double newValue = tab[i][tab[i].length - 1] / tab[i][enteringVariable];
            if (newValue > 0 && newValue < bestValue) {
                bestIdx = i;
                bestValue = newValue;
            }
        }
        return bestIdx;

    }

    // computes dest[i] = dest[i] - val * src[i]
    private static void subtractAndMultiplyRows(final double[][] tab, final int src, final int dest,
            final double val) {
        for (int i = 0; i < tab[src].length; i++) {
            tab[dest][i] -= val * tab[src][i];
        }
    }

    private static final class Food {
        private final double avgPricePerKilo;
        private final double avgWeight;
        private final double fatPercentage;
        private final double carbPercentage;
        private final double proteinPercentage;

        public Food(double avgPricePerKilo, double avgWeight, double fatPercentage, double carbPercentage,
                double proteinPercentage) {
            this.avgPricePerKilo = avgPricePerKilo;
            this.avgWeight = avgWeight;
            this.fatPercentage = fatPercentage;
            this.carbPercentage = carbPercentage;
            this.proteinPercentage = proteinPercentage;
        }

        public double price() {
            return avgPricePerKilo;
        }

        public double fats() {
            return avgWeight * fatPercentage;
        }

        public double proteins() {
            return avgWeight * proteinPercentage;
        }

        public double carbs() {
            return avgWeight * carbPercentage;
        }
    }

    public static void main(final String[] args) {
        if (args.length > 0) {
            System.out.println("This program does not need input arguments. Ignoring them.");
        }

        // the following data is reported by column
        // a banana weighs 125 grams, of which: 0.3% fats, 23% carbs, 1.1% proteins
        // an apple weighs 100 grams, of which: 0.2% fats, 14% carbs, 0.3% proteins
        // a carrot weighs 60 grams, of which: 0.2% fats, 9.6% carbs, 0.9% proteins
        // a pizza weighs 900 grams, of which: 10% fats, 33% carbs, 11% proteins
        // a beef steak weighs 250 grams, of which: 19% fats, 0% carbs, 25% proteins
        // a banana costs 2.39€ per 1 kg
        // an apple costs 2.99€ per 1 kg
        // a carrot costs 2.69€ per 1 kg
        // a pizza costs 5€ per 1 kg
        // a beef steak costs 15.49€ per 1 kg
        final Food banana = new Food(2.39, 125, 0.003, 0.23, 0.011);
        final Food apple = new Food(2.99, 100, 0.002, 0.14, 0.003);
        final Food carrot = new Food(2.69, 60, 0.002, 0.096, 0.009);
        final Food pizza = new Food(5, 900, 0.1, 0.33, 0.11);
        final Food steak = new Food(15.49, 250, 0.19, 0, 0.25);
        final double calories = 2500;

        // This matrix represents the system of linear inequalities.
        // the "naive" constraints (x,y,z >= 0) are implicit
        final double[][] system = new double[][] {
                // these constraints are the rows
                // sum of all calories must be >= 2000 and <= 3000 (average adult male)
                // sum of all carbs must be >= 45% and <= 55% of all daily calories
                // sum of all proteins must be >= 10% and <= 35% of all daily calories
                // sum of all fats must be >= 20% and <= 35% of all daily calories

                { banana.carbs(), apple.carbs(), carrot.carbs(), pizza.carbs(), steak.carbs(), 0.55 * calories },
                { -banana.carbs(), -apple.carbs(), -carrot.carbs(), -pizza.carbs(), -steak.carbs(), -0.45 * calories },
                { banana.proteins(), apple.proteins(), carrot.proteins(), pizza.proteins(), steak.proteins(),
                        0.35 * calories },
                { -banana.proteins(), -apple.proteins(), -carrot.proteins(), -pizza.proteins(), -steak.proteins(),
                        -0.1 * calories },
                { banana.fats(), apple.fats(), carrot.fats(), pizza.fats(), steak.fats(), 0.35 * calories },
                { -banana.fats(), -apple.fats(), -carrot.fats(), -pizza.fats(), -steak.fats(), -0.2 * calories },
        };

        // the function to be maximized (if you want a minimization objective function,
        // put all negative weights instead of positive)
        final double[] function = new double[] { -banana.price(), -apple.price(), -carrot.price(), -pizza.price(),
                -steak.price() };

        final boolean useBlandRule = true;

        // input checks
        if (system.length == 0) {
            throw new IllegalArgumentException("There are no constraints in this problem.");
        }

        final int nVariables = system[0].length - 1;
        for (int i = 1; i < system.length; i++) {
            if (system[i].length != nVariables + 1) {
                throw new IllegalArgumentException(String.format(
                        "The %d-th constraint has a different number of variables form the first constraint.", i + 1));
            }
        }

        if (function.length == 0) {
            throw new IllegalArgumentException("There are no variables in the optimization function.");
        }
        if (function.length != nVariables) {
            throw new IllegalArgumentException(String.format(
                    "The optimization function has a different number of parameters (%d) than the number of available variables (%d).",
                    function.length, nVariables));
        }
        // end input check

        // printing the problem
        {
            System.out.println("Given that:");
            for (int i = 0; i < nVariables; i++) {
                for (int j = 0; j < nVariables; j++) {
                    if (i == j) {
                        System.out.printf("%9s%c ", String.format("%+3.3f", 1.0), 'a' + i);
                    } else {
                        System.out.print("           ");
                    }
                }
                System.out.println(">= 0");
            }
            for (int i = 0; i < system.length; i++) {
                for (int j = 0; j < nVariables; j++) {
                    if (system[i][j] != 0) {
                        System.out.printf("%9s%c ", String.format("%+3.3f", system[i][j]), 'a' + j);
                    } else {
                        System.out.print("           ");
                    }
                }
                System.out.printf("<= %+3.3f\n", system[i][nVariables]);
            }

            System.out.println();
            System.out.println("We want to maximize: ");
            System.out.print("Z = ");
            for (int i = 0; i < function.length; i++) {
                if (i < nVariables) {
                    System.out.printf("%+3.3f%c ", function[i], 'a' + i);
                } else {
                    System.out.printf("%+3.3f\n", function[i]);
                }
            }
            System.out.println();
        }

        // standardizing the problem
        final double[][] standardSystem = new double[system.length][nVariables + 1 + system.length];
        for (int i = 0; i < system.length; i++) {
            for (int j = 0; j < nVariables; j++) {
                standardSystem[i][j] = system[i][j];
            }
            standardSystem[i][nVariables + i] = 1;
            standardSystem[i][standardSystem[i].length - 1] = system[i][nVariables];
        }

        // printing the standardized system
        {
            System.out.println();
            System.out.println("The standardized system is:");
            for (int i = 0; i < standardSystem.length; i++) {
                for (int j = 0; j < standardSystem[i].length - 1; j++) {
                    if (standardSystem[i][j] != 0) {
                        System.out.printf("%9s%c ", String.format("%+3.3f", standardSystem[i][j]), 'a' + j);
                    } else {
                        System.out.print("           ");
                    }
                }
                System.out.printf(" = %+3.3f\n", standardSystem[i][standardSystem[i].length - 1]);
            }

            System.out.println();
            System.out.println("We want to maximize: ");
            System.out.print("Z ");
            for (int i = 0; i < function.length; i++) {
                if (i < nVariables) {
                    System.out.printf("%+3.3f%c ", -function[i], 'a' + i);
                } else {
                    System.out.printf("%+3.3f ", -function[i]);
                }
            }
            System.out.println(" = 0");
        }

        // Constructing the tableau
        final double[][] tableau = new double[system.length + 1][standardSystem[0].length + 1];
        for (int i = 0; i < system.length; i++) {
            for (int j = 0; j < standardSystem[i].length - 1; j++) {
                tableau[i][j] = standardSystem[i][j];
            }
            tableau[i][tableau[i].length - 1] = standardSystem[i][standardSystem[i].length - 1];
        }
        for (int j = 0; j < function.length; j++) {
            tableau[tableau.length - 1][j] = -function[j];
        }
        tableau[tableau.length - 1][tableau[0].length - 2] = 1;

        // printing the tableau
        System.out.println();
        System.out.println("The tableau is:");
        printTableau(tableau);
        System.out.println();

        // solving the problem
        int iteration = 1;
        boolean isOptimal;

        while (true) {
            isOptimal = isOptimalSolution(tableau);

            System.out.printf("Iteration: %d\n", iteration);
            // printTableau(tableau);
            System.out.println("Current solution:");
            for (int i = 0; i < tableau[0].length - 2; i++) {
                final char variableName = (char) ('a' + i);

                if (tableau[tableau.length - 1][i] != 0) {
                    System.out.printf("  %c = 0\n", variableName);
                } else {
                    for (int j = 0; j < tableau.length - 1; j++) {
                        if (tableau[j][i] == 0) {
                            continue;
                        }
                        System.out.printf("  %c = %+3.3f\n", variableName, tableau[j][tableau[j].length - 1]);
                        break;
                    }
                }
            }
            System.out.printf("  Z = %+3.3f\n", tableau[tableau.length - 1][tableau[0].length - 1]);
            System.out.println(isOptimal ? "Optimal solution" : "Not the optimal solution");
            if (isOptimal) {
                break;
            }

            int enteringVariable;
            if (useBlandRule) {
                enteringVariable = findWithBlandRule(tableau);
            } else {
                enteringVariable = findBestEnteringVariable(tableau);
            }
            System.out.printf("Entering variable: %c\n", 'a' + enteringVariable);

            final boolean finiteSolutionExists = checkFiniteSolutionExists(tableau, enteringVariable);
            if (!finiteSolutionExists) {
                System.out.printf(
                        "The selected column (%d) contains only negative entries. This means that a finite solution does not exist and the solution polyhedron is unbounded in the direction of the objective function.\n",
                        enteringVariable);
                break;
            }

            final int minRowIdx = findMinRow(tableau, enteringVariable);
            // making sure that the pivot is 1 by dividing all its row by the pivot value
            {
                final double pivotValue = tableau[minRowIdx][enteringVariable];
                for (int i = 0; i < tableau[minRowIdx].length; i++) {
                    tableau[minRowIdx][i] /= pivotValue;
                }
            }

            // zero-ing all rows in the `enteringVariable column`
            {
                // now the pivot is 1
                for (int i = 0; i < tableau.length; i++) {
                    if (i == minRowIdx) {
                        continue;
                    }
                    subtractAndMultiplyRows(tableau, minRowIdx, i,
                            tableau[i][enteringVariable]);
                }
            }

            System.out.println();
            iteration++;
        }
    }
}