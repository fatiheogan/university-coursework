package DSA.Sorting;

import java.util.Comparator;
import java.util.Random;

/**
 * A class that implements the quicksort algorithm by extending GTUSorter.
 * This class sorts an array in ascending order using the quicksort algorithm,
 * with an optimization that switches to another sorter for small partitions (less than PARTITION_LIMIT).
 * A random pivot selection is used to improve average-case performance.
 */
public class GTUQuickSort extends GTUSorter {
    private GTUSorter smallPartitionSorter; // Sorter for small partitions
    private static final int PARTITION_LIMIT = 10; // Threshold for small partitions
    private Random random; // Random generator for pivot selection

    /**
     * Constructs a GTUQuickSort instance with a specified sorter for small partitions
     * and a random number generator for pivot selection.
     * <p>
     * <b>Time Complexity:</b> O(1).
     * <b>Space Complexity:</b> O(1).
     * @param smallPartitionSorter The sorter to use for small partitions.
     */
    public GTUQuickSort(GTUSorter smallPartitionSorter) {
        if (smallPartitionSorter == null) {
            this.smallPartitionSorter = new GTUQuickSort(); // Default to QuickSort
        } else {
            this.smallPartitionSorter = smallPartitionSorter;
        }
        this.random = new Random();
    }

    /**
     * Constructs a GTUQuickSort instance with a GTUInsertSort for small partitions.
     * <p>
     * <b>Time Complexity:</b> O(1).
     * <b>Space Complexity:</b> O(1).
     * @param smallPartitionSorter The GTUInsertSort instance to use for small partitions.
     */
    public GTUQuickSort(GTUInsertSort smallPartitionSorter) {
        this.smallPartitionSorter = smallPartitionSorter;
        this.random = new Random();
    }

    /**
     * Constructs a GTUQuickSort instance with a GTUSelectSort for small partitions.
     * <p>
     * <b>Time Complexity:</b> O(1).
     * <b>Space Complexity:</b> O(1).
     * @param smallPartitionSorter The GTUSelectSort instance to use for small partitions.
     */
    public GTUQuickSort(GTUSelectSort smallPartitionSorter) {
        this.smallPartitionSorter = smallPartitionSorter;
        this.random = new Random();
    }

    /**
     * Constructs a GTUQuickSort instance without a specified sorter for small partitions.
     * It will continue using quicksort for small partitions.
     * <p>
     * <b>Time Complexity:</b> O(1).
     * <b>Space Complexity:</b> O(1).
     */
    public GTUQuickSort() {
        this.smallPartitionSorter = null; // Default to QuickSort
        this.random = new Random();
    }

    /**
     * Sorts a subarray between start (inclusive) and end (exclusive) using the quicksort algorithm.
     * For partitions smaller than PARTITION_LIMIT, it switches to the specified sorter for better performance.
     * <p>
     * <b>Time Complexity:</b> O(n log n) on average, O(n^2) in worst case, where n is the size of the subarray (end - start).
     * <b>Space Complexity:</b> O(log n) due to the recursion stack.
     * @param <T> Any type that can be compared using the provided comparator.
     * @param arr Array to be sorted.
     * @param start First index (inclusive) of the subarray to sort.
     * @param end Last index (exclusive) of the subarray to sort.
     * @param comparator A comparator to determine the order of elements.
     */
    @Override
    protected <T> void sort(T[] arr, int start, int end, Comparator<T> comparator) {
        if (end - start <= 1) return; // Base case: subarray of size 0 or 1

        // Switch to small partition sorter if applicable
        if (end - start <= PARTITION_LIMIT && smallPartitionSorter != null) {
            smallPartitionSorter.sort(arr, start, end, comparator);
            return;
        }

        // Partition and recursively sort
        int pivotIndex = partition(arr, start, end, comparator);
        sort(arr, start, pivotIndex, comparator);
        sort(arr, pivotIndex + 1, end, comparator);
    }

    /**
     * Partitions the subarray around a randomly chosen pivot and returns the pivot's final position.
     * Random pivot selection helps avoid worst-case scenarios for already sorted or nearly sorted arrays.
     * <p>
     * <b>Time Complexity:</b> O(n) where n is the size of the subarray (end - start).
     * <b>Space Complexity:</b> O(1).
     * @param <T> Any type that can be compared using the provided comparator.
     * @param arr Array to be partitioned.
     * @param start First index (inclusive) of the subarray.
     * @param end Last index (exclusive) of the subarray.
     * @param comparator A comparator to determine the order of elements.
     * @return The final position of the pivot.
     */
    private <T> int partition(T[] arr, int start, int end, Comparator<T> comparator) {
        // Choose a random pivot
        int randomIndex = start + random.nextInt(end - start);
        swap(arr, randomIndex, end - 1);
        T pivot = arr[end - 1];
        int i = start - 1;

        // Partition around the pivot
        for (int j = start; j < end - 1; j++) {
            if (comparator.compare(arr[j], pivot) <= 0) {
                i++;
                swap(arr, i, j);
            }
        }
        swap(arr, i + 1, end - 1);
        return i + 1;
    }

    /**
     * Swaps two elements in the array.
     * <p>
     * <b>Time Complexity:</b> O(1).
     * <b>Space Complexity:</b> O(1).
     * @param arr Array containing the elements to swap.
     * @param i Index of the first element.
     * @param j Index of the second element.
     * @param <T> The type of elements in the array.
     */
    private <T> void swap(T[] arr, int i, int j) {
        T temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}