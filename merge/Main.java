import java.util.Set;
import java.util.HashSet;
import java.util.Map;
import java.util.HashMap;
import java.io.Console;

public final class Main {

    private static enum Element {
        EARTH, WATER, AIR, FIRE, LAVA, STEAM, DUST, FOAM, MUD, SEA, OCEAN, ROCK, SPARKS, SWAMP, MOUNTAIN, VOLCANO,
        GEYSER, WAVES, TSUNAMI, CLOUDS, RAIN, HEAT, SOIL, PLANT, WIND, EROSION, SAND, DESERT, BEACH, ISLAND,
        ARCHIPELAGO, CONTINENT, PLANET, ARRAKIS, PANGEA
    }

    private static record Pair(Element first, Element second) {
    }

    private static final Map<Pair, Element> combinations = new HashMap<>();
    private static final Set<Element> unlockedElements = new HashSet<>();

    private static void clearScreen() {
        System.out.print("\033[2J");
    }

    private static Element askElement(final Console console, final String message) {
        Element el = null;
        do {
            System.out.print(message);
            final String line = console.readLine().strip();
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

    public static void main(final String[] args) {
        if (args.length > 0) {
            System.out.println("Command-line arguments not needed. Ignoring them.");
        }

        unlockedElements.add(Element.EARTH);
        unlockedElements.add(Element.WATER);
        unlockedElements.add(Element.AIR);
        unlockedElements.add(Element.FIRE);

        combinations.put(new Pair(Element.EARTH, Element.FIRE), Element.LAVA);
        combinations.put(new Pair(Element.EARTH, Element.EARTH), Element.ROCK);
        combinations.put(new Pair(Element.ROCK, Element.ROCK), Element.MOUNTAIN);
        combinations.put(new Pair(Element.ROCK, Element.WATER), Element.EROSION);
        combinations.put(new Pair(Element.ROCK, Element.AIR), Element.EROSION);
        combinations.put(new Pair(Element.MOUNTAIN, Element.EROSION), Element.ROCK);
        combinations.put(new Pair(Element.MOUNTAIN, Element.LAVA), Element.VOLCANO);
        combinations.put(new Pair(Element.MOUNTAIN, Element.EARTH), Element.ISLAND);
        combinations.put(new Pair(Element.ISLAND, Element.ISLAND), Element.ARCHIPELAGO);
        combinations.put(new Pair(Element.ISLAND, Element.EARTH), Element.CONTINENT);
        combinations.put(new Pair(Element.CONTINENT, Element.CONTINENT), Element.PANGEA);
        combinations.put(new Pair(Element.CONTINENT, Element.OCEAN), Element.PLANET);
        combinations.put(new Pair(Element.WATER, Element.FIRE), Element.STEAM);
        combinations.put(new Pair(Element.STEAM, Element.EARTH), Element.GEYSER);
        combinations.put(new Pair(Element.EARTH, Element.AIR), Element.DUST);
        combinations.put(new Pair(Element.DUST, Element.DUST), Element.SAND);
        combinations.put(new Pair(Element.SAND, Element.SAND), Element.DESERT);
        combinations.put(new Pair(Element.SAND, Element.SEA), Element.BEACH);
        combinations.put(new Pair(Element.PLANET, Element.DESERT), Element.ARRAKIS);
        combinations.put(new Pair(Element.SAND, Element.OCEAN), Element.BEACH);
        combinations.put(new Pair(Element.DUST, Element.FIRE), Element.SPARKS);
        combinations.put(new Pair(Element.WATER, Element.AIR), Element.FOAM);
        combinations.put(new Pair(Element.FOAM, Element.AIR), Element.CLOUDS);
        combinations.put(new Pair(Element.WATER, Element.CLOUDS), Element.RAIN);
        combinations.put(new Pair(Element.FIRE, Element.AIR), Element.HEAT);
        combinations.put(new Pair(Element.WATER, Element.EARTH), Element.MUD);
        combinations.put(new Pair(Element.WATER, Element.MUD), Element.SWAMP);
        combinations.put(new Pair(Element.WATER, Element.WATER), Element.SEA);
        combinations.put(new Pair(Element.SEA, Element.FOAM), Element.WAVES);
        combinations.put(new Pair(Element.WATER, Element.SEA), Element.OCEAN);
        combinations.put(new Pair(Element.WAVES, Element.WATER), Element.TSUNAMI);
        combinations.put(new Pair(Element.MUD, Element.HEAT), Element.SOIL);
        combinations.put(new Pair(Element.SOIL, Element.RAIN), Element.PLANT);
        combinations.put(new Pair(Element.AIR, Element.AIR), Element.WIND);

        for (final Pair p : combinations.keySet()) {
            if (!p.first().equals(p.second()) && combinations.containsKey(new Pair(p.second(), p.first()))) {
                throw new IllegalArgumentException(
                        String.format("The combinations %s + %s = %s and %s + %s = %s are redundant", p.first(),
                                p.second(), combinations.get(p), p.second(), p.first(),
                                combinations.get(new Pair(p.second(), p.first()))));
            }
            if (p.first().equals(p.second()) && p.first().equals(combinations.get(p))) {
                throw new IllegalArgumentException(String.format("The combination %s + %s = %s is useless", p.first(),
                        p.second(), combinations.get(p)));
            }
        }

        final Console console = System.console();

        while (true) {
            clearScreen();
            System.out.printf("You have unlocked %,d elements:\n", unlockedElements.size());
            unlockedElements.stream().map(Element::name).sorted().forEach(x -> System.out.printf("%s ", x));
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
            System.out.println("\nPress ENTER to continue...");
            console.readLine();
        }
    }
}
