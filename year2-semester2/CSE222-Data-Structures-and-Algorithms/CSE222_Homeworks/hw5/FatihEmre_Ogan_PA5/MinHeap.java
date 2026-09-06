/**
   * A generic MinHeap for MyPriorityQueue, storing elements in a binary heap.
   * Parents are smaller than children.
   * @param <T> the element type, must implement Comparable
   */
  public class MinHeap<T extends Comparable<T>> implements MyPriorityQueue<T> {
    private T[] heap; // Array to store heap elements
    private int size; // Current number of elements
    private static final int DEFAULT_CAPACITY = 10; // Default capacity

    /**
     * Creates an empty MinHeap with default capacity.
     * Time complexity: O(1)
     */
    @SuppressWarnings("unchecked")
    public MinHeap() {
        this.heap = (T[]) new Comparable[DEFAULT_CAPACITY];
        this.size = 0;
    }

    /**
     * Adds an element to the heap, keeping heap property.
     * Time complexity: O(log n)
     * @param t the element to add
     */
    @Override
    public void add(T t) {
        if (size == heap.length) {
            resize();
        }
        heap[size] = t;
        siftUp(size);
        size++;
    }

    /**
     * Removes and returns the smallest element (root).
     * Time complexity: O(log n)
     * @return the smallest element, or null if empty
     */
    @Override
    public T poll() {
        if (isEmpty()) {
            return null;
        }
        T result = heap[0];
        heap[0] = heap[size - 1];
        heap[size - 1] = null;
        size--;
        if (size > 0) {
            siftDown(0);
        }
        return result;
    }

    /**
     * Checks if the heap is empty.
     * Time complexity: O(1)
     * @return true if empty, false otherwise
     */
    @Override
    public Boolean isEmpty() {
        return size == 0;
    }

    /**
     * Resizes the heap array when full.
     * Time complexity: O(n)
     */
    @SuppressWarnings("unchecked")
    private void resize() {
        T[] newHeap = (T[]) new Comparable[heap.length * 2];
        for (int i = 0; i < heap.length; i++) {
            newHeap[i] = heap[i];
        }
        heap = newHeap;
    }

    /**
     * Moves element at index up to correct position.
     * Time complexity: O(log n)
     * @param index the index to sift up
     */
    private void siftUp(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;
            if (heap[index].compareTo(heap[parent]) < 0) {
                swap(index, parent);
                index = parent;
            } else {
                break;
            }
        }
    }

    /**
     * Moves element at index down to correct position.
     * Time complexity: O(log n)
     * @param index the index to sift down
     */
    private void siftDown(int index) {
        while (true) {
            int left = 2 * index + 1;
            int right = 2 * index + 2;
            int minIndex = index;

            if (left < size && heap[left] != null && heap[left].compareTo(heap[minIndex]) < 0) {
                minIndex = left;
            }
            if (right < size && heap[right] != null && heap[right].compareTo(heap[minIndex]) < 0) {
                minIndex = right;
            }
            if (minIndex == index) {
                break;
            }
            swap(index, minIndex);
            index = minIndex;
        }
    }

    /**
     * Swaps two elements in the heap.
     * Time complexity: O(1)
     * @param i first element index
     * @param j second element index
     */
    private void swap(int i, int j) {
        T temp = heap[i];
        heap[i] = heap[j];
        heap[j] = temp;
    }
}