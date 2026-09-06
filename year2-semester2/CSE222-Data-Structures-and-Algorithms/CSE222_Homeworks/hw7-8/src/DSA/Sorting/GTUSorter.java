package DSA.Sorting;

import java.util.Comparator;

/**
 * A generic abstract class for sorting arrays using a comparator.
 * This class provides a framework for sorting algorithms by defining a public sort method
 * that delegates to an abstract protected sort method, which must be implemented by subclasses.
 * It supports sorting of any type that can be compared using a provided comparator.
 */
public abstract class GTUSorter {
    /**
     * Constructs a new GTUSorter instance with default settings.
     * This abstract class serves as a base for sorting algorithms, requiring subclasses to implement the sort method.
     * <p>
     * <b>Time Complexity:</b> O(1) for construction.
     * <b>Space Complexity:</b> O(1).
     */
    public GTUSorter() {
        // Default constructor
    }

    /**
     * Sorts the entire array in ascending order using the provided comparator.
     * This method delegates to the abstract sort method, passing the full range of the array.
     * Time Complexity: Depends on the implementation in the subclass (e.g., O(n^2) for insertion sort, O(n log n) for quicksort).
     * Space Complexity: Depends on the implementation in the subclass (e.g., O(1) for in-place sorting).
     * 
     * @param <T> The type of elements in the array.
     * @param arr The array to be sorted.
     * @param comparator A comparator to determine the order of elements.
     */
    public <T> void sort(T[] arr, Comparator<T> comparator) {
        sort(arr, 0, arr.length, comparator);
    }

    /**
     * Sorts a subarray between start (inclusive) and end (exclusive) in ascending order.
     * This method must be implemented by subclasses to provide the specific sorting algorithm.
     * Time Complexity: Depends on the implementation in the subclass.
     * Space Complexity: Depends on the implementation in the subclass.
     * 
     * @param <T> The type of elements in the array.
     * @param arr The array to be sorted.
     * @param start The first index (inclusive) of the subarray to sort.
     * @param end The last index (exclusive) of the subarray to sort.
     * @param comparator A comparator to determine the order of elements.
     */
    protected abstract <T> void sort(T[] arr, int start, int end, Comparator<T> comparator);
}