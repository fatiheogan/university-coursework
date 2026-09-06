package DSA.Sorting;

import java.util.Comparator;

/**
 * A class that implements the selection sort algorithm by extending GTUSorter.
 * This class sorts an array in ascending order using the selection sort algorithm,
 * handling a subarray between specified start and end indices. The sorting is performed
 * in-place using a provided comparator for generic types.
 */
public class GTUSelectSort extends GTUSorter {
    /**
     * Constructs a new GTUSelectSort instance with default settings.
     * Initializes the sorter for use with any comparable type.
     * <p>
     * <b>Time Complexity:</b> O(1) for construction.
     * <b>Space Complexity:</b> O(1).
     */
    public GTUSelectSort() {
        // No initialization needed
    }

    /**
     * Sorts a subarray between start (inclusive) and end (exclusive) using the selection sort algorithm.
     * The algorithm repeatedly selects the smallest element from the unsorted portion and swaps it
     * with the first element of the unsorted portion.
     * 
     * @param <T> Any type that can be compared using the provided comparator.
     * @param arr Array to be sorted.
     * @param start First index (inclusive) of the subarray to sort.
     * @param end Last index (exclusive) of the subarray to sort.
     * @param comparator A comparator to determine the order of elements.
     * Time Complexity: O(n^2) where n is the size of the subarray (end - start).
     * Space Complexity: O(1) as it performs in-place sorting.
     */
    @Override
    protected <T> void sort(T[] arr, int start, int end, Comparator<T> comparator) {
        for (int i = start; i < end - 1; i++) {
            int minIndex = i;
            for (int j = i + 1; j < end; j++) {
                if (comparator.compare(arr[j], arr[minIndex]) < 0) {
                    minIndex = j;
                }
            }
            // Swap the found minimum element with the first element of the unsorted portion
            T temp = arr[i];
            arr[i] = arr[minIndex];
            arr[minIndex] = temp;
        }
    }
}