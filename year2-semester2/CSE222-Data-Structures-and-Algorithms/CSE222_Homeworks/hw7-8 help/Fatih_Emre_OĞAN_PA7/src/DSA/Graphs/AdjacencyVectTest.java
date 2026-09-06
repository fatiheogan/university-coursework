package DSA.Graphs;

public class AdjacencyVectTest {
    public static void testAddRemoveContains() {
        // Test verisi
        AdjacencyVect vect = new AdjacencyVect(5);
        vect.add(1);
        boolean contains = vect.contains(1);
        vect.remove(1);
        boolean afterRemove = vect.contains(1);

        // Sonucu yaz
        System.out.println("Contains 1 after add: " + contains);
        System.out.println("Contains 1 after remove: " + afterRemove);
        System.out.println("Size: " + vect.size());

        // Manuel kontrol
        boolean passed = contains && !afterRemove && vect.size() == 0;
        System.out.println("Test " + (passed ? "PASSED" : "FAILED"));
    }

    public static void main(String[] args) {
        testAddRemoveContains();
    }
}