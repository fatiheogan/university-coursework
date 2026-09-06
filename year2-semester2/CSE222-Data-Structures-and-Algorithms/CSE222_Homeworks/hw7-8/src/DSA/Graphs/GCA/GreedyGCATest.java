package DSA.Graphs.GCA;

import DSA.Graphs.MatrixGraph;
import DSA.Sorting.GTUSelectSort;

public class GreedyGCATest {
    public static void testSolve() {
        // Test verisi: 3 vertex, 0-1 arası kenar
        MatrixGraph graph = new MatrixGraph(3);
        graph.setEdge(0, 1);
        GTUSelectSort sorter = new GTUSelectSort();
        GCASolution solution = GreedyGCA.solve(graph, sorter);

        // Renkleri bul
        int color0 = -1, color1 = -1;
        for (int i = 0; i < solution.colorNum(); i++) { // getNumColors() yerine colorNum()
            if (solution.getColorVertices(i).contains(0)) color0 = i;
            if (solution.getColorVertices(i).contains(1)) color1 = i;
        }

        // Çakışma kontrolü
        boolean noConflict = color0 != color1 && color0 != -1 && color1 != -1;

        // Sonucu yaz
        System.out.println("Color of 0: " + color0);
        System.out.println("Color of 1: " + color1);
        System.out.println("No conflict: " + noConflict);

        // Manuel kontrol
        boolean passed = noConflict;
        System.out.println("Test " + (passed ? "PASSED" : "FAILED"));
    }

    public static void main(String[] args) {
        testSolve();
    }
}