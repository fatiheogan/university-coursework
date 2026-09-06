package DSA.Sorting;

import java.util.Comparator;

public class GTUInsertSortTest {
    public static void testSort() {
        // Test verisi
        Integer[] arr = {4, 3, 2, 1};
        Integer[] expected = {1, 2, 3, 4};

        // Sorter oluştur
        GTUInsertSort sorter = new GTUInsertSort();
        sorter.sort(arr, Comparator.naturalOrder());

        // Sonucu kontrol et ve yaz
        System.out.print("Sorted array: ");
        for (int i : arr) System.out.print(i + " ");
        System.out.println();
        System.out.print("Expected: ");
        for (int i : expected) System.out.print(i + " ");
        System.out.println();

        // Manuel kontrol için uyarı
        boolean passed = true;
        for (int i = 0; i < arr.length; i++) {
            if (!arr[i].equals(expected[i])) {
                passed = false;
                break;
            }
        }
        System.out.println("Test " + (passed ? "PASSED" : "FAILED"));
    }

    public static void main(String[] args) {
        testSort();
    }
}