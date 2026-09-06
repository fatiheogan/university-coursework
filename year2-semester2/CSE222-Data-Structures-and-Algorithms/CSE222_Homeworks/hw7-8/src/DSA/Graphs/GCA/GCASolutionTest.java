package DSA.Graphs.GCA;

public class GCASolutionTest {
    public static void testColoring() {
        // Test verisi
        GCASolution solution = new GCASolution(3);
        solution.setColor(0, 1);
        solution.addColor();
        solution.setColor(1, 2);

        // Sonucu yaz
        System.out.println("Vertices in color 1: " + solution.getColorVertices(1));
        System.out.println("Vertices in color 2: " + solution.getColorVertices(2));

        // Manuel kontrol
        boolean passed = solution.getColorVertices(1).contains(0) && solution.getColorVertices(2).contains(1);
        System.out.println("Test " + (passed ? "PASSED" : "FAILED"));
    }

    public static void main(String[] args) {
        testColoring();
    }
}