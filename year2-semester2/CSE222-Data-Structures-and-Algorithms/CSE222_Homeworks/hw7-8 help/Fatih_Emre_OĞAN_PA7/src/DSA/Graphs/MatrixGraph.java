package DSA.Graphs;

import java.util.Collection;

/**
 * A graph implementation using an adjacency matrix, implementing the GTUGraph interface.
 * Represents an undirected unweighted graph where edges are stored in a boolean matrix.
 */
public class MatrixGraph implements GTUGraph {
    private boolean[][] adjMatrix;
    private int size;

    /**
     * Constructs a new MatrixGraph with the given size.
     * Time Complexity: O(V^2) for matrix initialization.
     * Space Complexity: O(V^2) for the adjacency matrix.
     * 
     * @param size The number of vertices in the graph.
     */
    public MatrixGraph(int size) {
        this.size = size;
        this.adjMatrix = new boolean[size][size];
    }

    /**
     * Sets an edge between vertices v1 and v2 in the graph.
     * Since the graph is undirected, the edge is set in both directions.
     * Time Complexity: O(1)
     * Space Complexity: O(1)
     * 
     * @param v1 First vertex ID.
     * @param v2 Second vertex ID.
     * @return True if the edge was successfully set, false if the vertices are invalid.
     */
    @Override
    public Boolean setEdge(int v1, int v2) {
        if (v1 < 0 || v1 >= size || v2 < 0 || v2 >= size) {
            return false;
        }
        adjMatrix[v1][v2] = true;
        adjMatrix[v2][v1] = true; // Undirected graph
        return true;
    }

    /**
     * Checks if an edge exists between vertices v1 and v2.
     * Time Complexity: O(1)
     * Space Complexity: O(1)
     * 
     * @param v1 First vertex ID.
     * @param v2 Second vertex ID.
     * @return True if the edge exists, false otherwise.
     */
    @Override
    public Boolean getEdge(int v1, int v2) {
        if (v1 < 0 || v1 >= size || v2 < 0 || v2 >= size) {
            return false;
        }
        return adjMatrix[v1][v2];
    }

    /**
     * Gets a collection of the neighbors of vertex v.
     * Time Complexity: O(V) where V is the number of vertices.
     * Space Complexity: O(V) for the neighbor list.
     * 
     * @param v Vertex ID.
     * @return The collection of neighbors.
     */
    @Override
    public Collection<Integer> getNeighbors(int v) {
        AdjacencyVect neighbors = new AdjacencyVect(size);
        if (v >= 0 && v < size) {
            for (int i = 0; i < size; i++) {
                if (adjMatrix[v][i]) {
                    neighbors.add(i);
                }
            }
        }
        return neighbors;
    }

    /**
     * Returns the size of the graph (number of vertices).
     * Time Complexity: O(1)
     * Space Complexity: O(1)
     * 
     * @return Size of the graph as an integer.
     */
    @Override
    public int size() {
        return size;
    }

    /**
     * Resets the graph with the given size.
     * Time Complexity: O(V^2) where V is the new size.
     * Space Complexity: O(V^2) for the adjacency matrix.
     * 
     * @param size New size of the graph (number of vertices).
     */
    @Override
    public void reset(int size) {
        this.size = size;
        this.adjMatrix = new boolean[size][size];
    }
}