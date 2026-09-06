package DSA.Sorting;

import java.util.Comparator;

/**
 * A class that implements the insertion sort algorithm by extending GTUSorter.
 * This class sorts an array in ascending order using the insertion sort algorithm,
 * handling a subarray between specified start and end indices.
 * The sorting is performed in-place using a provided comparator for generic types.
 */
public class GTUInsertSort extends GTUSorter {
    /**
     * Constructs a new GTUInsertSort instance with default settings.
     * No initialization is required for this implementation.
     * <p>
     * <b>Time Complexity:</b> O(1) for construction.
     * <b>Space Complexity:</b> O(1).
     */
    public GTUInsertSort() {
        // No initialization needed
    }

    /**
     * Sorts a subarray between start (inclusive) and end (exclusive) using the insertion sort algorithm.
     * The algorithm iterates through the array, inserting each element into its correct
     * position in the sorted portion by shifting larger elements to the right.
     * 
     * @param <T> Any type that can be compared using the provided comparator.
     * @param arr Array to be sorted.
     * @param start First index (inclusive) of the subarray to sort.
     * @param end Last index (exclusive) of the subarray to sort.
     * @param comparator A comparator to determine the order of elements.
     * <p>
     * <b>Time Complexity:</b> O(n^2) where n is the size of the subarray (end - start).
     * <b>Space Complexity:</b> O(1) as it performs in-place sorting.
     */
    @Override
    protected <T> void sort(T[] arr, int start, int end, Comparator<T> comparator) {
        for (int i = start + 1; i < end; i++) {
            T key = arr[i];
            int j = i - 1;
            while (j >= start && comparator.compare(arr[j], key) > 0) {
                arr[j + 1] = arr[j];
                j--;
            }
            arr[j + 1] = key;
        }
    }
}