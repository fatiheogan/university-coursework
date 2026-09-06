package DSA.Graphs;

import java.util.Collection;
import java.util.Iterator;

/**
 * A class representing an adjacency vector for a graph's vertex.
 * Implements the Collection interface to manage a list of adjacent vertices efficiently.
 */
public class AdjacencyVect implements Collection<Integer> {
    private boolean[] vector; // Array to store adjacency information
    private int size; // Number of adjacent vertices

    /**
     * Constructs a new AdjacencyVect with the specified capacity.
     * Initializes a boolean array to represent adjacency and sets the size to zero.
     * @param capacity The initial capacity of the adjacency vector (number of vertices in the graph).
     */
    public AdjacencyVect(int capacity) {
        this.vector = new boolean[capacity];
        this.size = 0;
    }

    /**
     * Returns the number of adjacent vertices.
     * @return The number of elements in this collection.
     */
    @Override
    public int size() {
        return size;
    }

    /**
     * Checks if the adjacency vector is empty.
     * @return True if there are no adjacent vertices, false otherwise.
     */
    @Override
    public boolean isEmpty() {
        return size == 0;
    }

    /**
     * Checks if the specified vertex is an adjacent vertex.
     * @param o The object to check (expected to be an Integer representing a vertex).
     * @return True if the vertex is adjacent, false otherwise.
     */
    @Override
    public boolean contains(Object o) {
        if (!(o instanceof Integer)) {
            return false;
        }
        int index = (Integer) o;
        return index >= 0 && index < vector.length && vector[index];
    }

    /**
     * Returns an iterator over the adjacent vertices.
     * @return An iterator over the elements in this collection.
     */
    @Override
    public Iterator<Integer> iterator() {
        return new Iterator<Integer>() {
            private int currentIndex = -1; // Tracks the current position in iteration

            /**
             * Checks if there are more adjacent vertices to iterate over.
             * @return True if there is a next element, false otherwise.
             */
            @Override
            public boolean hasNext() {
                for (int i = currentIndex + 1; i < vector.length; i++) {
                    if (vector[i]) {
                        return true;
                    }
                }
                return false;
            }

            /**
             * Returns the next adjacent vertex in the iteration.
             * @return The next vertex index, or null if no more elements exist.
             */
            @Override
            public Integer next() {
                for (int i = currentIndex + 1; i < vector.length; i++) {
                    if (vector[i]) {
                        currentIndex = i;
                        return i;
                    }
                }
                return null; // No more elements
            }
        };
    }

    /**
     * Returns an array containing all adjacent vertices.
     * @return An array of vertex indices.
     */
    @Override
    public Object[] toArray() {
        Object[] result = new Object[size];
        int index = 0;
        for (int i = 0; i < vector.length; i++) {
            if (vector[i]) {
                result[index++] = i;
            }
        }
        return result;
    }

    /**
     * Returns an array containing all adjacent vertices, using the provided array if possible.
     * @param a The array into which the elements are to be stored, if it is big enough.
     * @param <T> The type of the array.
     * @return An array containing the adjacent vertices.
     */
    @SuppressWarnings("unchecked")
    @Override
    public <T> T[] toArray(T[] a) {
        if (a.length < size) {
            a = (T[]) new Object[size]; // Create new array if provided one is too small
        }
        int index = 0;
        for (int i = 0; i < vector.length; i++) {
            if (vector[i]) {
                a[index++] = (T) Integer.valueOf(i);
            }
        }
        if (index < a.length) {
            a[index] = null; // Null-terminate if array is larger
        }
        return a;
    }

    /**
     * Adds a vertex to the adjacency vector.
     * @param e The vertex index to add.
     * @return True if the vertex was added, false otherwise.
     */
    @Override
    public boolean add(Integer e) {
        if (e != null && e >= 0 && e < vector.length && !vector[e]) {
            vector[e] = true;
            size++;
            return true;
        }
        return false;
    }

    /**
     * Removes a vertex from the adjacency vector.
     * @param o The vertex index to remove (expected to be an Integer).
     * @return True if the vertex was removed, false otherwise.
     */
    @Override
    public boolean remove(Object o) {
        if (o instanceof Integer) {
            int index = (Integer) o;
            if (index >= 0 && index < vector.length && vector[index]) {
                vector[index] = false;
                size--;
                return true;
            }
        }
        return false;
    }

    /**
     * Checks if all vertices in the specified collection are adjacent.
     * @param c The collection of vertices to check.
     * @return True if all vertices are adjacent, false otherwise.
     */
    @Override
    public boolean containsAll(Collection<?> c) {
        for (Object o : c) {
            if (!contains(o)) {
                return false;
            }
        }
        return true;
    }

    /**
     * Adds all vertices from the specified collection to the adjacency vector.
     * @param c The collection of vertices to add.
     * @return True if the adjacency vector was modified, false otherwise.
     */
    @Override
    public boolean addAll(Collection<? extends Integer> c) {
        boolean modified = false;
        for (Integer e : c) {
            if (add(e)) {
                modified = true;
            }
        }
        return modified;
    }

    /**
     * Removes all vertices in the specified collection from the adjacency vector.
     * @param c The collection of vertices to remove.
     * @return True if the adjacency vector was modified, false otherwise.
     */
    @Override
    public boolean removeAll(Collection<?> c) {
        boolean modified = false;
        for (Object o : c) {
            if (remove(o)) {
                modified = true;
            }
        }
        return modified;
    }

    /**
     * Retains only the vertices that are in the specified collection.
     * @param c The collection of vertices to retain.
     * @return True if the adjacency vector was modified, false otherwise.
     */
    @Override
    public boolean retainAll(Collection<?> c) {
        boolean modified = false;
        for (int i = 0; i < vector.length; i++) {
            if (vector[i] && !c.contains(i)) {
                vector[i] = false;
                size--;
                modified = true;
            }
        }
        return modified;
    }

    /**
     * Removes all vertices from the adjacency vector.
     */
    @Override
    public void clear() {
        for (int i = 0; i < vector.length; i++) {
            vector[i] = false;
        }
        size = 0;
    }
}