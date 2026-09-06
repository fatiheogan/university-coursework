/**
 * Interface for a priority queue that supports adding, polling, and checking emptiness.
 * @param <T> the type of elements, must implement Comparable
 */
interface MyPriorityQueue<T extends Comparable<T>> {
    /**
     * Adds an element to the priority queue.
     * @param t the element to add
     */
    void add(T t);

    /**
     * Removes and returns the highest priority element.
     * @return the highest priority element, or null if empty
     */
    T poll();

    /**
     * Checks if the priority queue is empty.
     * @return true if the queue is empty, false otherwise
     */
    Boolean isEmpty();
}