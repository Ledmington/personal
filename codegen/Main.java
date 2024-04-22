package codegen;

import java.util.stream.Stream;
import java.util.List;
import java.util.ArrayList;
import java.util.random.RandomGenerator;
import java.util.random.RandomGeneratorFactory;

public final class Main {

    private static final RandomGenerator rng = RandomGeneratorFactory.getDefault().create(System.nanoTime());

    private static enum Register {
        A(0),
        B(1),
        C(2),
        D(3);

        public static Register random() {
            return Register.values()[rng.nextInt(Register.values().length)];
        }

        public final int id;

        Register(int id) {
            this.id = id;
        }
    }

    private static enum Opcode {
        MOV,
        ADD,
        SUB,
        MUL,
        DIV,
        SQRT,
        PRINT;

        public static Opcode random() {
            return Opcode.values()[rng.nextInt(Opcode.values().length)];
        }
    }

    private static final class Instruction {
        public final Opcode opcode;
        public final Register[] regs;

        public static Instruction random() {
            final Opcode opcode = Opcode.random();
            return switch (opcode) {
                case PRINT -> new Instruction(opcode, 1);
                case MOV, SQRT -> new Instruction(opcode, 2);
                case ADD, DIV, MUL, SUB -> new Instruction(opcode, 3);
                default -> throw new IllegalArgumentException("Unexpected value: " + opcode);
            };
        }

        public Instruction(final Opcode opcode, final Register[] regs) {
            this.opcode = opcode;
            this.regs = regs;
        }

        public Instruction(final Opcode opcode, final int nRegs) {
            this(opcode, new Register[nRegs]);
            for (int i = 0; i < nRegs; i++) {
                regs[i] = Register.random();
            }
        }

        public String toString() {
            final StringBuilder sb = new StringBuilder();
            sb.append(opcode.name());
            sb.append(' ');
            sb.append(regs[0].name());
            for (int i = 1; i < regs.length; i++) {
                sb.append(',');
                sb.append(regs[i].name());
            }
            return sb.toString();
        }
    }

    private static final class Solution {
        public final List<Instruction> code;

        public Solution(final List<Instruction> code) {
            this.code = code;
        }

        public String toString() {
            final StringBuilder sb = new StringBuilder();
            for (int i = 0; i < code.size(); i++) {
                sb.append(code.get(i)).append('\n');
            }
            return sb.toString();
        }
    }

    private static double evaluateSingle(final Solution s, final double x) {
        /*
         * This setup is to try to find a simple example of fast exponentiation.
         * We place x in a register and we want to compute
         * 5*(x+1), 10*(x+2) and 15*(x+3) in the least amount of code possible.
         * 
         * Note: this objective function can be extended to include average
         * latencies for each instruction execution and also incorporate
         * the internal pipeline architecture of the CPU to optimize the usage
         * of internal compute units.
         * By applying these changes, this algorithm could be used to improve
         * existing micro-kernels to exploit a higher percentage of CPU resources.
         */
        final double[] registers = new double[Register.values().length];
        registers[0] = x;
        registers[1] = 0.0;
        registers[2] = 0.0;
        registers[3] = 0.0;
        final double expectedResult1 = 5.0 * (x + 1.0);
        final double expectedResult2 = 10.0 * (x + 2.0);
        final double expectedResult3 = 15.0 * (x + 3.0);

        final List<Double> output = new ArrayList<>();

        for (int i = 0; i < s.code.size(); i++) {
            final Instruction inst = s.code.get(i);
            final Register[] r = inst.regs;
            switch (inst.opcode) {
                case MOV:
                    registers[r[0].id] = registers[r[1].id];
                    break;
                case ADD:
                    registers[r[0].id] = registers[r[1].id] + registers[r[2].id];
                    break;
                case SUB:
                    registers[r[0].id] = registers[r[1].id] - registers[r[2].id];
                    break;
                case MUL:
                    registers[r[0].id] = registers[r[1].id] * registers[r[2].id];
                    break;
                case DIV:
                    registers[r[0].id] = registers[r[1].id] / registers[r[2].id];
                    break;
                case SQRT:
                    registers[r[0].id] = Math.sqrt(registers[r[1].id]);
                    break;
                case PRINT:
                    output.add(registers[r[0].id]);
                    break;
                default:
                    throw new IllegalArgumentException("Unknown opcode");
            }
        }

        double score = 0.0;
        final int expectedOutputSize = 3;

        if (output.size() < expectedOutputSize) {
            return Double.POSITIVE_INFINITY;
        }

        score += (double) (output.size() - expectedOutputSize) * 100.0;
        score += (double) s.code.size() * 100.0;
        if (output.size() >= 1) {
            final double diff1 = expectedResult1 - output.get(0);
            score += diff1 * diff1;
            if (output.size() >= 2) {
                final double diff2 = expectedResult2 - output.get(1);
                score += diff2 * diff2;
                if (output.size() >= 3) {
                    final double diff3 = expectedResult3 - output.get(2);
                    score += diff3 * diff3;
                }
            }

            // "we want the output as soon as possible"
            for (int i = 0; i < s.code.size(); i++) {
                if (s.code.get(i).opcode == Opcode.PRINT) {
                    score += (double) (i + 1);
                }
            }
        }

        return score;
    }

    private static double evaluate(final Solution s, final double[] dataset) {
        double total = 0.0;
        for (int i = 0; i < dataset.length; i++) {
            total += evaluateSingle(s, dataset[i]);
        }
        return total / (double) dataset.length;
    }

    private static Solution generateRandomNeighbor(final Solution s) {
        /*
         * There are 3 options:
         * - add an instruction (choose which and choose where to insert)
         * - remove an instruction
         * - modify an instruction
         */
        enum Choice {
            INSERT, REMOVE, MODIFY
        }
        final int maxCodeLen = 20;
        final int codeLen = s.code.size();

        final Choice choice;
        if (codeLen == 0) {
            choice = Choice.INSERT;
        } else if (codeLen >= maxCodeLen) {
            choice = Choice.REMOVE;
        } else {
            choice = Choice.values()[rng.nextInt(0, Choice.values().length)];
        }

        final int instructionIndex = (codeLen == 0) ? 0 : rng.nextInt(0, codeLen);
        final List<Instruction> newCode = new ArrayList<>(s.code);

        switch (choice) {
            case REMOVE -> {
                newCode.remove(instructionIndex);
            }
            case MODIFY -> {
                if (rng.nextBoolean()) {
                    // completely replace the instruction
                    newCode.add(Instruction.random());
                } else {
                    // change one operand of the instruction
                    final Instruction inst = newCode.get(instructionIndex);
                    final Register[] newRegs = new Register[inst.regs.length];
                    System.arraycopy(inst.regs, 0, newRegs, 0, inst.regs.length);
                    final int registerIndex = rng.nextInt(0, inst.regs.length);
                    Register newReg;
                    do {
                        newReg = Register.random();
                    } while (newReg == inst.regs[registerIndex]);
                    newRegs[registerIndex] = newReg;
                    final Instruction newInstruction = new Instruction(inst.opcode, newRegs);
                    newCode.set(instructionIndex, newInstruction);
                }
            }
            case INSERT -> {
                final Instruction newInstruction = Instruction.random();
                newCode.add(instructionIndex, newInstruction);
            }
        }

        return new Solution(newCode);
    }

    private static double boltzmann(final double deltaEnergy, final double temperature) {
        final double x = -deltaEnergy / temperature;
        if (x >= 0.0) {
            return 1.0;
        }
        return Math.exp(x);

    }

    private static void simulatedAnnealing() {
        final int datasetLength = 1000;
        final double[] dataset = new double[datasetLength];
        for (int i = 0; i < datasetLength; i++) {
            dataset[i] = rng.nextDouble(-10.0, 10.0);
        }
        Solution best = new Solution(Stream.generate(Instruction::random).limit(10).toList());
        double bestScore = evaluate(best, dataset);
        System.out.printf("Initial solution (%.6f):\n", bestScore);
        System.out.println(best.toString());
        System.out.println(" --- ");
        final int maxIterations = 100_000;

        for (int i = 0; i < maxIterations; i++) {
            final Solution current = generateRandomNeighbor(best);

            final double score = evaluate(current, dataset);
            final double temperature = 1.0 -
                    ((double) (i + 1) / (double) (maxIterations));
            final double deltaEnergy = score - bestScore;
            if (score < bestScore || boltzmann(deltaEnergy, temperature) >= rng.nextDouble()) {
                System.out.printf("Found new best solution (%.6f):\n", score);
                System.out.println(current.toString());
                System.out.println(" --- ");
                best = current;
                bestScore = score;
            }
        }
    }

    public static void main(final String[] args) {
        simulatedAnnealing();
    }
}
