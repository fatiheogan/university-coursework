package DSA.Graphs;

import java.util.Collection;

public class MatrixGraphTest {
    public static void testEdgeAndNeighbors() {
        // Test verisi
        int size = 3;
        MatrixGraph graph = new MatrixGraph(size);

        // Kenar ekle
        graph.setEdge(0, 1);
        boolean edge01 = graph.getEdge(0, 1);
        boolean edge10 = graph.getEdge(1, 0);
        Collection<Integer> neighbors = graph.getNeighbors(0);

        // Sonucu yaz
        System.out.println("Edge (0,1): " + edge01);
        System.out.println("Edge (1,0): " + edge10);
        System.out.print("Neighbors of 0: ");
        for (int neighbor : neighbors) System.out.print(neighbor + " ");
        System.out.println();

        // Manuel kontrol
        boolean passed = edge01 && edge10 && neighbors.contains(1);
        System.out.println("Test " + (passed ? "PASSED" : "FAILED"));
    }

    public static void main(String[] args) {
        testEdgeAndNeighbors();
    }
}