package DSA.Graphs.GCA;

import java.util.Collection;
import DSA.Graphs.GTUGraph;
import DSA.Sorting.GTUSorter;

/**
 * A greedy graph coloring algorithm implementation.
 * Vertices are sorted by degree (number of neighbors) in descending order,
 * and each vertex is assigned the smallest possible color that does not conflict with its neighbors.
 */
public class GreedyGCA {
    /**
     * Constructs a new GreedyGCA instance with default settings.
     * This class is used to perform greedy graph coloring based on vertex degrees.
     * <p>
     * <b>Time Complexity:</b> O(1) for construction.
     * <b>Space Complexity:</b> O(1).
     */
    public GreedyGCA() {
        // Default constructor
    }

    /**
     * A helper class representing a vertex with its ID and neighbors for greedy coloring.
     * Implements Comparable to sort vertices by degree and ID.
     */
    public static class GreedyVertex implements Comparable<GreedyVertex> {
        private int id; // Vertex ID
        private Collection<Integer> neighbors; // Adjacent vertices

        /**
         * Constructs a new GreedyVertex with the specified ID.
         * @param id The vertex ID.
         */
        public GreedyVertex(int id) {
            this.id = id;
        }

        /**
         * Compares this vertex with another based on degree (descending) and ID (ascending).
         * @param o The other GreedyVertex to compare with.
         * @return A negative integer, zero, or a positive integer if this vertex is less than,
         *         equal to, or greater than the specified vertex.
         */
        @Override
        public int compareTo(GreedyVertex o) {
            int degreeCompare = o.neighbors.size() - this.neighbors.size();
            if (degreeCompare == 0) {
                return this.id - o.id;
            }
            return degreeCompare;
        }

        /**
         * Returns the vertex ID.
         * @return The vertex ID.
         */
        public int getID() {
            return id;
        }

        /**
         * Sets the neighbors of this vertex.
         * @param neighbors The collection of neighboring vertices.
         */
        public void setNeighbors(Collection<Integer> neighbors) {
            this.neighbors = neighbors;
        }

        /**
         * Returns the neighbors of this vertex.
         * @return The collection of neighboring vertices.
         */
        public Collection<Integer> getNeighbors() {
            return neighbors;
        }
    }

    /**
     * Solves the graph coloring problem using a greedy approach.
     * Vertices are sorted by degree in descending order using the provided sorter,
     * and each vertex is assigned the smallest possible color that does not conflict with its neighbors.
     * <p>
     * <b>Time Complexity:</b> O(V log V + V^2), where V is the number of vertices.
     * <b>Space Complexity:</b> O(V^2) for the graph and solution.
     * @param graph The graph to be colored.
     * @param sorter The sorter used to sort vertices by degree.
     * @return The graph coloring solution.
     */
    public static GCASolution solve(GTUGraph graph, GTUSorter sorter) {
        // Initialize vertices array with their neighbors
        GreedyVertex[] vertices = new GreedyVertex[graph.size()];
        for (int i = 0; i < graph.size(); i++) {
            var newVertex = new GreedyVertex(i);
            newVertex.setNeighbors(graph.getNeighbors(i));
            vertices[i] = newVertex;
        }

        // Sort vertices by degree (descending)
        sorter.sort(vertices, (v1, v2) -> v2.compareTo(v1));

        // Color each vertex greedily
        GCASolution solution = new GCASolution(graph.size());
        Boolean isColored;
        for (GreedyVertex vertex : vertices) {
            isColored = false;
            for (int i = 0; i < solution.colorNum(); i++) {
                // Check if the current color can be used (no conflicts with neighbors)
                boolean disjoint = true;
                Collection<Integer> colorVertices = solution.getColorVertices(i);
                for (Integer neighbor : vertex.getNeighbors()) {
                    if (colorVertices.contains(neighbor)) {
                        disjoint = false;
                        break;
                    }
                }
                if (disjoint) {
                    solution.setColor(vertex.getID(), i);
                    isColored = true;
                    break;
                }
            }

            // If no existing color can be used, add a new color
            if (!isColored) {
                Integer newColor = solution.addColor();
                solution.setColor(vertex.getID(), newColor);
            }
        }

        return solution;
    }
}