package DSA.Graphs.GCA;

import java.util.ArrayList;
import java.util.Collection;

public class GreedyVertexTest {
    public static void testCompareTo() {
        // Test verisi
        Collection<Integer> neighbors = new ArrayList<>();
        neighbors.add(2);

        // GreedyVertex’leri tam nitelikli adla oluştur
        GreedyGCA.GreedyVertex v1 = new GreedyGCA.GreedyVertex(0);
        GreedyGCA.GreedyVertex v2 = new GreedyGCA.GreedyVertex(1);
        v1.setNeighbors(neighbors); // 1 komşu
        v2.setNeighbors(new ArrayList<>()); // 0 komşu

        // Karşılaştırma
        int result = v1.compareTo(v2);
        boolean passed = result < 0; // v1’in derecesi (1) > v2’nin derecesi (0), descending order

        // Sonucu yaz
        System.out.println("Compare result: " + result);
        System.out.println("Test " + (passed ? "PASSED" : "FAILED"));
    }

    public static void main(String[] args) {
        testCompareTo();
    }
}