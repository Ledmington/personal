package diet;

import java.util.Objects;

public final class Main {

    private static void printTableau(final double[][] tab) {
        for (int i = 0; i < tab.length; i++) {
            for (int j = 0; j < tab[i].length; j++) {
                System.out.printf("%+9.3f ", tab[i][j]);
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

    private static record Food(String name, double avgPricePerKilo, double avgWeight, double fatPercentage,
            double carbPercentage,
            double proteinPercentage) {

        public Food {
            Objects.requireNonNull(name);
            if (name.isBlank()) {
                throw new IllegalArgumentException("Empty name.");
            }
            if (avgPricePerKilo <= 0.0) {
                throw new IllegalArgumentException("Non-positive average price per kilo.");
            }
            // NOTE: weight is in grams
            if (avgWeight <= 0.0) {
                throw new IllegalArgumentException("Non-positive average weight.");
            }
            if (fatPercentage <= 0.0 || fatPercentage >= 1.0) {
                throw new IllegalArgumentException("Invalid fat percentage.");
            }
            if (carbPercentage <= 0.0 || carbPercentage >= 1.0) {
                throw new IllegalArgumentException("Invalid carbohydrates percentage.");
            }
            if (proteinPercentage <= 0.0 || proteinPercentage >= 1.0) {
                throw new IllegalArgumentException("Invalid protein percentage.");
            }
            if (fatPercentage + carbPercentage + proteinPercentage >= 1.0) {
                throw new IllegalArgumentException("Invalid sum of percentages.");
            }
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

    private static char getVariableName(final int variableIndex) {
        return (char) ('a' + variableIndex);
    }

    public static void main(final String[] args) {
        if (args.length > 0) {
            System.out.println("This program does not need input arguments. Ignoring them.");
        }

        final Food[] foods = {
                new Food("banana", 2.39, 125, 0.003, 0.23, 0.011),
                new Food("apple", 2.99, 100, 0.002, 0.14, 0.003),
                new Food("carrot", 2.69, 60, 0.002, 0.096, 0.009),
                new Food("pizza", 5, 900, 0.1, 0.33, 0.11),
                new Food("steak", 15.49, 250, 0.19, 0.001, 0.25)
        };
        final int numFoods = foods.length;

        final double totalCalories = 2500;
        final double maxCalories = 3000;
        final double minCalories = 2000;
        final double maxCarbohydrates = 0.55 * totalCalories;
        final double minCarbohydrates = 0.45 * totalCalories;
        final double maxProteins = 0.35 * totalCalories;
        final double minProteins = 0.1 * totalCalories;
        final double maxFats = 0.35 * totalCalories;
        final double minFats = 0.2 * totalCalories;

        // This matrix represents the system of linear inequalities.
        // the "naive" constraints (x,y,z >= 0) are implicit
        final double[][] system = new double[6][numFoods + 1];

        // "Max Carbohydrates" constraint
        for (int i = 0; i < numFoods; i++) {
            system[0][i] = foods[i].carbs();
        }
        system[0][numFoods] = maxCarbohydrates;

        // "Min Carbohydrates" constraint
        for (int i = 0; i < numFoods; i++) {
            system[1][i] = -foods[i].carbs();
        }
        system[1][numFoods] = -minCarbohydrates;

        // "Max Proteins" constraint
        for (int i = 0; i < numFoods; i++) {
            system[2][i] = foods[i].proteins();
        }
        system[2][numFoods] = maxProteins;

        // "Min Proteins" constraint
        for (int i = 0; i < numFoods; i++) {
            system[3][i] = -foods[i].proteins();
        }
        system[3][numFoods] = -minProteins;

        // "Max Fats" constraint
        for (int i = 0; i < numFoods; i++) {
            system[4][i] = foods[i].fats();
        }
        system[4][numFoods] = maxFats;

        // "Min Fats" constraint
        for (int i = 0; i < numFoods; i++) {
            system[5][i] = -foods[i].fats();
        }
        system[5][numFoods] = -minFats;

        // the function to be maximized (if you want a minimization objective function,
        // put all negative weights instead of positive)
        final double[] function = new double[numFoods];
        for (int i = 0; i < numFoods; i++) {
            function[i] = -foods[i].avgPricePerKilo();
        }

        System.out.println("Input data:");
        System.out.println(" NAME      €/kg   %carb     %fat    %protein");
        for (int i = 0; i < numFoods; i++) {
            System.out.printf(" %-7s %6.2f  %6.3f%%  %6.3f%%   %6.3f%%  (%c)%n", foods[i].name(),
                    foods[i].avgPricePerKilo(),
                    foods[i].carbPercentage() * 100.0, foods[i].fatPercentage() * 100.0,
                    foods[i].proteinPercentage() * 100.0, getVariableName(i));
        }
        System.out.println();
        System.out.printf("Total calories from carbohydrates: [ %6.1f ; %6.1f ]%n", minCarbohydrates, maxCarbohydrates);
        System.out.printf("Total calories from fats         : [ %6.1f ; %6.1f ]%n", minFats, maxFats);
        System.out.printf("Total calories from proteins     : [ %6.1f ; %6.1f ]%n", minProteins, maxProteins);
        System.out.println();

        final boolean useBlandRule = true;

        // input checks
        if (system.length == 0) {
            throw new IllegalArgumentException("There are no constraints in this problem.");
        }

        final int numVariables = numFoods;
        for (int i = 1; i < system.length; i++) {
            if (system[i].length != numVariables + 1) {
                throw new IllegalArgumentException(String.format(
                        "The %d-th constraint has a different number of variables form the first constraint.", i + 1));
            }
        }

        if (function.length == 0) {
            throw new IllegalArgumentException("There are no variables in the optimization function.");
        }
        if (function.length != numVariables) {
            throw new IllegalArgumentException(String.format(
                    "The optimization function has a different number of parameters (%d) than the number of available variables (%d).",
                    function.length, numVariables));
        }
        // end input check

        // printing the problem
        {
            System.out.println("Given that:");
            for (int i = 0; i < numVariables; i++) {
                for (int j = 0; j < numVariables; j++) {
                    if (i == j) {
                        System.out.printf("%9s%c ", String.format("%+3.3f", 1.0), 'a' + i);
                    } else {
                        System.out.print("           ");
                    }
                }
                System.out.println(">= 0");
            }
            for (int i = 0; i < system.length; i++) {
                for (int j = 0; j < numVariables; j++) {
                    if (system[i][j] != 0) {
                        System.out.printf("%9s%c ", String.format("%+3.3f", system[i][j]), 'a' + j);
                    } else {
                        System.out.print("           ");
                    }
                }
                System.out.printf("<= %+3.3f\n", system[i][numVariables]);
            }

            System.out.println();
            System.out.println("We want to maximize: ");
            System.out.print("Z = ");
            for (int i = 0; i < function.length; i++) {
                if (i < numVariables) {
                    System.out.printf("%+3.3f%c ", function[i], 'a' + i);
                } else {
                    System.out.printf("%+3.3f\n", function[i]);
                }
            }
            System.out.println();
        }

        // standardizing the problem
        final double[][] standardSystem = new double[system.length][numVariables + 1 + system.length];
        for (int i = 0; i < system.length; i++) {
            for (int j = 0; j < numVariables; j++) {
                standardSystem[i][j] = system[i][j];
            }
            standardSystem[i][numVariables + i] = 1;
            standardSystem[i][standardSystem[i].length - 1] = system[i][numVariables];
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
                if (i < numVariables) {
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
                final char variableName = getVariableName(i);

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