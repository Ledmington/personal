import java.util.Set;
import java.util.stream.Collectors;
import java.util.HashSet;
import java.util.Map;
import java.util.Arrays;
import java.util.HashMap;
import java.io.Console;

public final class Main {

    private static enum Element {
        EARTH, WATER, AIR, FIRE, LAVA, STEAM, DUST, FOAM, MUD, SEA, OCEAN, ROCK, SPARKS, SWAMP, MOUNTAIN, VOLCANO,
        GEYSER, WAVES, TSUNAMI, CLOUDS, RAIN, HEAT, SOIL, PLANT, FOREST, WIND, EROSION, SAND, DESERT, BEACH, ISLAND,
        ARCIPELAGO, CONTINENT, PLANET, SOLAR_SYSTEM, GALAXY, GALAXY_CLUSTER, UNIVERSE, ARRAKIS, PANGEA, GLASS, METAL,
        WINDOW
    }

    private static record Pair(Element first, Element second) {
    }

    private static final Map<Pair, Element> combinations = new HashMap<>();
    private static final Set<Element> unlockedElements = new HashSet<>();

    private static void clearScreen() {
        System.out.print("\033[2J");
        System.out.print("\033[H");
        System.out.flush();
    }

    private static Element askElement(final Console console, final String message) {
        Element el = null;
        do {
            System.out.print(message);
            final String line = console.readLine().strip().replace(' ', '_');
            if (line.isEmpty() || line.isBlank()) {
                continue;
            }
            for (final Element e : Element.values()) {
                if (unlockedElements.contains(e) && line.equalsIgnoreCase(e.name())) {
                    el = e;
                    break;
                }
            }
            if (el == null) {
                System.out.printf("No element exists with name '%s'. Try again.\n", line);
            }
        } while (el == null);
        return el;
    }

    private static void checkRedundantCombinations() {
        for (final Pair p : combinations.keySet()) {
            final Element a = p.first();
            final Element b = p.second();
            if (!a.equals(b) && combinations.containsKey(new Pair(b, a))) {
                throw new IllegalArgumentException(
                        String.format("The combinations %s + %s = %s and %s + %s = %s are redundant.", a,
                                b, combinations.get(p), p.second(), a,
                                combinations.get(new Pair(b, a))));
            }
        }
    }

    private static void checkUselessCombinations() {
        for (final Pair p : combinations.keySet()) {
            final Element a = p.first();
            final Element b = p.second();
            if (a.equals(b) && a.equals(combinations.get(p))) {
                throw new IllegalArgumentException(String.format("The combination %s + %s = %s is useless.", a,
                        b, combinations.get(p)));
            }
        }
    }

    private static void checkUnreachableElements() {
        final Set<Element> visited = new HashSet<>();
        visited.addAll(unlockedElements);
        final Set<Element> newOnes = new HashSet<>();

        do {
            newOnes.clear();
            for (final Element a : visited) {
                for (final Element b : visited) {
                    final Pair x = new Pair(a, b);
                    final Pair y = new Pair(b, a);
                    if (combinations.containsKey(x) && !visited.contains(combinations.get(x))) {
                        newOnes.add(combinations.get(x));
                    } else if (combinations.containsKey(y) && !visited.contains(combinations.get(y))) {
                        newOnes.add(combinations.get(y));
                    }
                }
            }
            visited.addAll(newOnes);
        } while (newOnes.size() >= 1);

        if (visited.size() != Element.values().length) {
            throw new IllegalArgumentException(String.format(
                    "These elements are not reachable from the starting set: %s.", Arrays.stream(Element.values())
                            .filter(e -> !visited.contains(e)).map(Element::name).collect(Collectors.joining(", "))));
        }
    }

    private static void combine(final Element a, final Element b, final Element output) {
        combinations.put(new Pair(a, b), output);
    }

    private static boolean isComplete(final Element x) {
        for (final Pair p : combinations.keySet()) {
            final Element a = p.first();
            final Element b = p.second();
            if ((a.equals(x) || b.equals(x)) && !unlockedElements.contains(combinations.get(p))) {
                return false;
            }
        }
        return true;
    }

    private static String getTime(final long ms) {
        String x = (ms % 1_000L) + " ms";
        final long s = ms / 1_000L;
        if (s == 0L) {
            return x;
        }
        x = (s % 60L) + " s, " + x;
        final long m = s / 60L;
        if (m == 0L) {
            return x;
        }
        x = (m % 60L) + " m, " + x;
        final long h = m / 60L;
        if (h == 0L) {
            return x;
        }
        return h + " h, " + x;
    }

    public static void main(final String[] args) {
        if (args.length > 0) {
            System.out.println("Command-line arguments not needed. Ignoring them.");
        }

        unlockedElements.add(Element.EARTH);
        unlockedElements.add(Element.WATER);
        unlockedElements.add(Element.AIR);
        unlockedElements.add(Element.FIRE);

        combine(Element.EARTH, Element.FIRE, Element.LAVA);
        combine(Element.EARTH, Element.EARTH, Element.ROCK);
        combine(Element.ROCK, Element.ROCK, Element.MOUNTAIN);
        combine(Element.ROCK, Element.WATER, Element.EROSION);
        combine(Element.ROCK, Element.AIR, Element.EROSION);
        combine(Element.ROCK, Element.EROSION, Element.SAND);
        combine(Element.FIRE, Element.ROCK, Element.METAL);
        combine(Element.MOUNTAIN, Element.EROSION, Element.ROCK);
        combine(Element.MOUNTAIN, Element.LAVA, Element.VOLCANO);
        combine(Element.MOUNTAIN, Element.EARTH, Element.ISLAND);
        combine(Element.ISLAND, Element.ISLAND, Element.ARCIPELAGO);
        combine(Element.ISLAND, Element.EARTH, Element.CONTINENT);
        combine(Element.CONTINENT, Element.CONTINENT, Element.PANGEA);
        combine(Element.CONTINENT, Element.OCEAN, Element.PLANET);
        combine(Element.PLANET, Element.PLANET, Element.SOLAR_SYSTEM);
        combine(Element.SOLAR_SYSTEM, Element.SOLAR_SYSTEM, Element.GALAXY);
        combine(Element.GALAXY, Element.GALAXY, Element.GALAXY_CLUSTER);
        combine(Element.GALAXY_CLUSTER, Element.GALAXY_CLUSTER, Element.UNIVERSE);
        combine(Element.WATER, Element.FIRE, Element.STEAM);
        combine(Element.STEAM, Element.EARTH, Element.GEYSER);
        combine(Element.EARTH, Element.AIR, Element.DUST);
        combine(Element.DUST, Element.DUST, Element.SAND);
        combine(Element.SAND, Element.SAND, Element.DESERT);
        combine(Element.SAND, Element.SEA, Element.BEACH);
        combine(Element.SAND, Element.FIRE, Element.GLASS);
        combine(Element.METAL, Element.GLASS, Element.WINDOW);
        combine(Element.PLANET, Element.DESERT, Element.ARRAKIS);
        combine(Element.SAND, Element.OCEAN, Element.BEACH);
        combine(Element.DUST, Element.FIRE, Element.SPARKS);
        combine(Element.WATER, Element.AIR, Element.FOAM);
        combine(Element.FOAM, Element.AIR, Element.CLOUDS);
        combine(Element.WATER, Element.CLOUDS, Element.RAIN);
        combine(Element.FIRE, Element.AIR, Element.HEAT);
        combine(Element.WATER, Element.EARTH, Element.MUD);
        combine(Element.WATER, Element.MUD, Element.SWAMP);
        combine(Element.WATER, Element.WATER, Element.SEA);
        combine(Element.SEA, Element.FOAM, Element.WAVES);
        combine(Element.WATER, Element.SEA, Element.OCEAN);
        combine(Element.WAVES, Element.WATER, Element.TSUNAMI);
        combine(Element.MUD, Element.HEAT, Element.SOIL);
        combine(Element.SOIL, Element.RAIN, Element.PLANT);
        combine(Element.PLANT, Element.PLANT, Element.FOREST);
        combine(Element.AIR, Element.AIR, Element.WIND);

        checkRedundantCombinations();
        checkUselessCombinations();
        checkUnreachableElements();

        final Console console = System.console();

        final long startMillis = System.currentTimeMillis();
        int totalAttempts = 0;

        while (true) {
            clearScreen();

            if (unlockedElements.size() == Element.values().length) {
                final long endMillis = System.currentTimeMillis();

                System.out.println();
                System.out.println("  VICTORY!");
                System.out.printf("Congratulations! You have unlocked all the %,d elements in this game.\n",
                        Element.values().length);
                System.out.println("Here are some statistics:");
                System.out.printf("  Total time: %s\n", getTime(endMillis - startMillis));
                System.out.printf("  Total combinations: %,d\n", totalAttempts);
                System.out.println();
                System.exit(0);
                break;
            }

            System.out.printf("You have unlocked %,d out of %,d elements.\n", unlockedElements.size(),
                    Element.values().length);
            {
                final Set<Element> nonTerminal = unlockedElements.stream().filter(x -> !isComplete(x))
                        .collect(Collectors.toSet());
                System.out.printf("There are %,d elements that still need experimentation:\n", nonTerminal.size());
                nonTerminal.stream().map(Element::name).sorted().forEach(s -> System.out.print(" " + s));
            }
            System.out.println();
            System.out.println();

            final Element firstElement = askElement(console, "What do you want to merge? ");
            System.out.printf("You have selected %s.\n", firstElement.name());

            final Element secondElement = askElement(console,
                    String.format("What do you want to merge %s with? ", firstElement.name()));
            System.out.printf("You have selected %s.\n", secondElement.name());

            if (combinations.containsKey(new Pair(firstElement, secondElement))
                    || combinations.containsKey(new Pair(secondElement, firstElement))) {
                final Element newElement = combinations.containsKey(new Pair(firstElement, secondElement))
                        ? combinations.get(new Pair(firstElement, secondElement))
                        : combinations.get(new Pair(secondElement, firstElement));
                if (!unlockedElements.contains(newElement)) {
                    System.out.printf("Congratulations! You have unlocked %s!\n", newElement);
                    unlockedElements.add(newElement);
                } else {
                    System.out.printf("These two elements produce %s but you have already unlocked it.\n", newElement);
                }
            } else {
                System.out.println("These two elements produce nothing.");
            }

            totalAttempts++;

            System.out.println("\nPress ENTER to continue...");
            console.readLine();
        }
    }
}
