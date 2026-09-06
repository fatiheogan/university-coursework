package Main;

import DSA.Sorting.GTUSelectSort;

public class TestScenarioTest {
    public static void testConstructor() {
        // TestScenario’yu Main üzerinden oluştur
        Main.TestScenario scenario = new Main.TestScenario("test.txt", new GTUSelectSort());
        String file = scenario.name; // name field'ına doğrudan erişim
        GTUSelectSort sorter = (GTUSelectSort) scenario.sorter; // Cast gerekebilir

        // Sonucu yaz
        System.out.println("File name: " + file);
        System.out.println("Sorter: " + (sorter != null));
        boolean passed = file.equals("test.txt") && sorter != null;
        System.out.println("Test " + (passed ? "PASSED" : "FAILED"));
    }

    public static void main(String[] args) {
        testConstructor();
    }
}