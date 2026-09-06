package DSA.Graphs.GCA;

import java.io.FileWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Arrays;

import DSA.Graphs.AdjacencyVect;

/**
 * A class representing a graph coloring solution.
 * Stores colors as collections of vertices and provides methods to manage and write the solution.
 */
public class GCASolution {
    private ArrayList<Collection<Integer>> colors; // List of vertex collections for each color
    private int vNum; // Number of vertices in the graph

    /**
     * Constructs a new GCASolution for a graph with the specified number of vertices.
     * @param vNum The number of vertices in the graph.
     */
    public GCASolution(int vNum) {
        colors = new ArrayList<>();
        this.vNum = vNum;
    }

    /**
     * Assigns a color to the specified vertex.
     * Expands the color list if the color index does not exist.
     * @param v The vertex ID to assign a color to.
     * @param c The color ID to assign.
     */
    public void setColor(int v, int c) {
        while (c >= colors.size()) {
            colors.add(new AdjacencyVect(vNum));
        }
        colors.get(c).add(v);
    }

    /**
     * Adds a new color to the solution and returns its ID.
     * @return The ID of the newly added color.
     */
    public Integer addColor() {
        colors.add(new AdjacencyVect(vNum));
        return colors.size() - 1;
    }

    /**
     * Returns the number of colors used in the solution.
     * @return The number of colors.
     */
    public Integer colorNum() {
        return colors.size();
    }

    /**
     * Returns the collection of vertices assigned to the specified color.
     * @param c The color ID.
     * @return The collection of vertices assigned to the color.
     */
    public Collection<Integer> getColorVertices(int c) {
        return colors.get(c);
    }

    /**
     * Writes the coloring solution to a file.
     * File format: First line is the number of vertices, second line is the number of colors,
     * followed by each color and its assigned vertices in ascending order.
     * <p>
     * <b>Time Complexity:</b> O(V log V), where V is the number of vertices, due to sorting.
     * <b>Space Complexity:</b> O(V) for the temporary array.
     * @param filePath The path to the output file.
     * @return True if the operation is successful, false otherwise.
     */
    public Boolean writeSolution(String filePath) {
        FileWriter writer;
        try {
            writer = new FileWriter(filePath);

            // Write number of vertices
            writer.write(String.format("%d\n", vNum));
            // Write number of colors
            writer.write(String.format("%d\n", colors.size()));

            // Write vertices for each color in ascending order
            for (int i = 0; i < colorNum(); i++) {
                Collection<Integer> vertices = getColorVertices(i);
                // Manually copy vertices to array for sorting
                Integer[] sortedVertices = new Integer[vertices.size()];
                int index = 0;
                for (Integer vertex : vertices) {
                    sortedVertices[index++] = vertex;
                }
                Arrays.sort(sortedVertices); // Sort vertices in ascending order
                for (int j = 0; j < sortedVertices.length; j++) {
                    writer.write(String.format("%d %d\n", i, sortedVertices[j]));
                }
            }

            writer.close();
            return true;
        } catch (Exception e) {
            System.err.printf("Error: %s\n", e.getMessage());
            return false;
        }
    }
}