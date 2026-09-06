package DSA.Sorting;

import java.util.Comparator;

public class GTUSelectSortTest {
    public static void testSort() {
        // Test verisi
        Integer[] arr = {5, 2, 8, 1};
        Integer[] expected = {1, 2, 5, 8};

        // Sorter oluştur
        GTUSelectSort sorter = new GTUSelectSort();
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