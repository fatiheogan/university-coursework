package DSA.Graphs;

import java.io.File;
import java.io.FileWriter;
import java.util.Collection;
import java.util.Scanner;

/**
 * A simple graph interface with basic operations for undirected unweighted graphs.
 */
public interface GTUGraph {
    /**
     * Sets an edge between vertices v1 and v2 in the graph.
     * Time Complexity: O(1) for adjacency matrix implementation.
     * Space Complexity: O(1)
     * 
     * @param v1 First vertex ID.
     * @param v2 Second vertex ID.
     * @return True if the edge was successfully set, false otherwise.
     */
    Boolean setEdge(int v1, int v2);

    /**
     * Checks if an edge exists between vertices v1 and v2.
     * Time Complexity: O(1) for adjacency matrix implementation.
     * Space Complexity: O(1)
     * 
     * @param v1 First vertex ID.
     * @param v2 Second vertex ID.
     * @return True if the edge exists, false otherwise.
     */
    Boolean getEdge(int v1, int v2);

    /**
     * Gets a collection of the neighbors of vertex v.
     * Time Complexity: O(V) for adjacency matrix implementation.
     * Space Complexity: O(V) for storing neighbors.
     * 
     * @param v Vertex ID.
     * @return The collection of neighbors.
     */
    Collection<Integer> getNeighbors(int v);

    /**
     * Returns the size of the graph (number of vertices).
     * Time Complexity: O(1)
     * Space Complexity: O(1)
     * 
     * @return Size of the graph as an integer.
     */
    int size();

    /**
     * Resets the graph with the given size.
     * Time Complexity: O(V^2) for adjacency matrix implementation.
     * Space Complexity: O(V^2) for the adjacency matrix.
     * 
     * @param size New size of the graph (number of vertices).
     */
    void reset(int size);

    /**
     * Reinitializes the graph using the information in the given file.
     * The file format should start with the number of vertices, followed by
     * edges listed as pairs of vertex IDs.
     * Time Complexity: O(V + E) where V is the number of vertices and E is the number of edges.
     * Space Complexity: O(V^2) for the graph structure.
     * 
     * @param filePath Path to the file.
     * @param graph Graph instance to reinitialize.
     */
    static void readGraph(String filePath, GTUGraph graph) {
        try {
            File file = new File(filePath);
            Scanner scanner = new Scanner(file);

            Integer size = Integer.parseInt(scanner.nextLine().strip());
            graph.reset(size);
    
            String[] line;
            while (scanner.hasNextLine()) {
                line = scanner.nextLine().strip().split(" ");
                Integer v1 = Integer.parseInt(line[0]);
                Integer v2 = Integer.parseInt(line[1]);
    
                graph.setEdge(v1, v2);
            }
    
            scanner.close();
        } catch (Exception e) {
            System.err.printf("Error: %s\n", e.getMessage());
            return;
        }
    }

    /**
     * Writes the information of the given graph to a file.
     * The file starts with the number of vertices, followed by edges listed
     * as pairs of vertex IDs (only i &lt; vertex to avoid duplicates in undirected graph).
     * Time Complexity: O(V + E) where V is the number of vertices and E is the number of edges.
     * Space Complexity: O(1) (excluding file I/O).
     * 
     * @param filePath Path to the file.
     * @param graph Graph instance to write.
     */
    static void writeGraph(String filePath, GTUGraph graph) {
        try {
            File file = new File(filePath);
            FileWriter writer = new FileWriter(file);

            writer.write(Integer.toString(graph.size()) + "\n");

            for (int i = 0; i < graph.size(); i++) {
                for (var vertex : graph.getNeighbors(i)) {
                    if (i < vertex) {
                        writer.write(String.format("%d %d\n", i, vertex));
                    }
                }
            }

            writer.close();
        } catch (Exception e) {
            System.err.printf("Error: %s\n", e.getMessage());
            return;
        }
    }
}