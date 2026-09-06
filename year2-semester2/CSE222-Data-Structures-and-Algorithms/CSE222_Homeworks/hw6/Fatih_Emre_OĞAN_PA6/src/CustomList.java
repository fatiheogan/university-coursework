import java.util.Iterator;
import java.util.NoSuchElementException;

// A custom list class that can store any type of elements (like a list of strings, integers, etc.)
public class CustomList<E> implements Iterable<E> {
    private Object[] elements;  // Array to store the elements
    private int size;          // Current number of elements in the list
    private int capacity;      // Total space in the array

    // Constructor: Creates an empty list with initial capacity of 10
    public CustomList() {  // No unchecked warning here, just initialization
        this.capacity = 10;
        this.elements = new Object[capacity];  // No cast, so no need for @SuppressWarnings
        this.size = 0;
    }

    // Adds a new element to the end of the list if it's not already in the list
    public void add(E element) {
        if (contains(element)) {  // Check if the element is already in the list
            return;  // If yes, don't add it
        }
        if (size >= capacity) {  // If the list is full, make it bigger
            resize();
        }
        elements[size++] = element;  // No cast needed, just assignment
    }

    // Gets the element at the given index
    @SuppressWarnings("unchecked")
    public E get(int index) {
        if (index < 0 || index >= size) {  // Check if index is valid
            throw new IndexOutOfBoundsException("Index: " + index + ", Size: " + size);
        }
        return (E) elements[index];  // Cast from Object to E, needs @SuppressWarnings
    }

    // Checks if an element is already in the list
    @SuppressWarnings("unchecked")
    public boolean contains(E element) {
        for (int i = 0; i < size; i++) {
            if (element == null) {
                if (elements[i] == null) {
                    return true;  // Found a match (both are null)
                }
            } else if (element.equals((E) elements[i])) {
                return true;  // Found a match
            }
        }
        return false;  // Element not found
    }

    // Returns the current number of elements in the list
    public int size() {
        return size;
    }

    // Makes the list bigger when it's full
    private void resize() {
        capacity *= 2;  // Double the capacity
        Object[] newElements = new Object[capacity];  // No cast, just new array
        for (int i = 0; i < size; i++) {
            newElements[i] = elements[i];  // No cast, just assignment
        }
        elements = newElements;  // No cast, just reference update
    }

    // Converts the list to a string (like [a, b, c]) for printing
    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder("[");
        for (int i = 0; i < size; i++) {
            sb.append(elements[i]);  // No cast, just append
            if (i < size - 1) {
                sb.append(", ");  // Add a comma between elements
            }
        }
        sb.append("]");
        return sb.toString();
    }

    // Returns an iterator to loop over the list
    @Override
    public Iterator<E> iterator() {
        return new CustomListIterator();
    }

    // Inner class to help loop over the list using an iterator
    private class CustomListIterator implements Iterator<E> {
        private int currentIndex = 0;  // Tracks the current position in the list

        // Checks if there are more elements to loop over
        @Override
        public boolean hasNext() {
            return currentIndex < size;
        }

        // Gets the next element in the list
        @SuppressWarnings("unchecked")
        @Override
        public E next() {
            if (!hasNext()) {  // If no more elements, throw an error
                throw new NoSuchElementException();
            }
            return (E) elements[currentIndex++];  // Cast from Object to E, needs @SuppressWarnings
        }
    }
}