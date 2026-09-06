package Main;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.Scanner;

import DSA.Graphs.GTUGraph;
import DSA.Graphs.GCA.GCASolution;
import DSA.Graphs.GCA.GreedyGCA;
import DSA.Graphs.MatrixGraph;
import DSA.Sorting.GTUInsertSort;
import DSA.Sorting.GTUQuickSort;
import DSA.Sorting.GTUSelectSort;
import DSA.Sorting.GTUSorter;

/**
 * Main class for CSE222 Homework 7-8.
 * Reads a graph from an input file, processes it using MatrixGraph and sorting algorithms,
 * and generates three output files for each test scenario: adjacency information,
 * sorted sums of edge pairs, and the graph coloring solution.
 */
public class Main {
    /**
     * Constructs a new Main instance with default settings.
     * This class serves as the entry point for the CSE222 Homework 7-8 program.
     * <p>
     * <b>Time Complexity:</b> O(1) for construction.
     * <b>Space Complexity:</b> O(1).
     */
    public Main() {
        // Default constructor
    }

    /**
     * A helper class representing a test scenario with a name and sorter.
     * Used to define different sorting strategies for testing.
     */
    public static class TestScenario {
        String name;
        GTUSorter sorter;

        /**
         * Constructs a new TestScenario.
         * @param name Name of the test scenario.
         * @param sorter The sorter to be used for this scenario.
         */
        TestScenario(String name, GTUSorter sorter) {
            this.name = name;
            this.sorter = sorter;
        }
    }

    /**
     * Main method to execute the homework program.
     * Reads command-line arguments, processes the graph, and generates output files.
     * @param args Command-line arguments: input file path and output directory path.
     * @throws IOException If an I/O error occurs during file operations.
     */
    public static void main(String[] args) throws IOException {
        // Check command-line arguments
        if (args.length != 2) {
            System.err.println("Usage: make run ARGS=\"<input file> <output directory ending with '/'>\"");
            System.exit(1);
        }

        String inputFile = args[0];
        String outputPath = args[1];

        // Create output directory if it does not exist
        File outputDir = new File(outputPath);
        if (!outputDir.exists()) {
            if (!outputDir.mkdirs()) {
                System.err.println("Error: Could not create output directory: " + outputPath);
                System.exit(1);
            }
        }

        // Define test scenarios
        ArrayList<TestScenario> scenarios = new ArrayList<>();
        scenarios.add(new TestScenario("MyInsertSort", new GTUInsertSort()));
        scenarios.add(new TestScenario("MyQuickSort", new GTUQuickSort()));
        scenarios.add(new TestScenario("MyQuickSortInsertSort", new GTUQuickSort(new GTUInsertSort())));
        scenarios.add(new TestScenario("MyQuickSortSelectSort", new GTUQuickSort(new GTUSelectSort())));
        scenarios.add(new TestScenario("MySelectSort", new GTUSelectSort()));

        // Process each scenario
        for (int i = 0; i < scenarios.size(); i++) {
            TestScenario scenario = scenarios.get(i);
            // Create and initialize graph
            MatrixGraph graph = new MatrixGraph(0); // Size will be set by readGraph
            GTUGraph.readGraph(inputFile, graph);

            // Output 1: Adjacency information (written once for all scenarios)
            if (i == 0) {
                GTUGraph.writeGraph(outputPath + "graph.txt", graph);
            }

            // Output 2: Sorted sums of edge pairs
            writeSortedSums(inputFile, outputPath + scenario.name + ".txt", scenario.sorter);

            // Output 3: Graph coloring solution
            GCASolution solution = GreedyGCA.solve(graph, scenario.sorter);
            solution.writeSolution(outputPath + scenario.name + ".color.txt");
        }
    }

    /**
     * Writes the sorted sums of edge pairs from the input file to an output file.
     * Reads edge pairs, computes their sums, sorts them in descending order, and writes to the file.
     * @param inputFile Path to the input file.
     * @param outputFile Path to the output file (e.g., GTUInsertSort.txt).
     * @param sorter The sorter to be used for sorting the sums.
     * @throws IOException If an I/O error occurs during file operations.
     * <p>
     * <b>Time Complexity:</b> O(E log E), where E is the number of edges.
     * <b>Space Complexity:</b> O(E) for storing the sums.
     */
    private static void writeSortedSums(String inputFile, String outputFile, GTUSorter sorter) throws IOException {
        // Read edge pairs and compute their sums
        ArrayList<Integer> sums = new ArrayList<>();
        File file = new File(inputFile);
        Scanner scanner = new Scanner(file);
        scanner.nextInt(); // Skip graph size
        while (scanner.hasNextInt()) {
            int v1 = scanner.nextInt();
            int v2 = scanner.nextInt();
            sums.add(v1 + v2);
        }
        scanner.close();

        // Sort the sums in descending order
        Integer[] sumsArray = new Integer[sums.size()];
        for (int i = 0; i < sums.size(); i++) {
            sumsArray[i] = sums.get(i);
        }
        sorter.sort(sumsArray, Comparator.reverseOrder());

        // Write sorted sums to file
        FileWriter writer = new FileWriter(outputFile);
        for (int i = 0; i < sumsArray.length; i++) {
            writer.write(sumsArray[i] + "\n");
        }
        writer.close();
    }
}