public class GTUHashSet<E> {
    private static final Object WORD = new Object();  // A dummy object to use as a value in the map
    private GTUHashMap<E, Object> map;  // Map to store elements with a dummy value

    // Constructor: Creates a new hash set using a hash map
    public GTUHashSet() {
        map = new GTUHashMap<>();
    }

    // Adds an element to the set
    public void add(E element) {
        map.put(element, WORD);  // Store the element with the dummy value
    }

    // Removes an element from the set
    public void remove(E element) {
        map.remove(element);  // Remove the element from the map
    }

    // Checks if an element is in the set
    public boolean contains(E element) {
        return map.containsKey(element);  // Return true if the element exists
    }

    // Returns the number of elements in the set
    public int size() {
        return map.size();  // Return the size of the map
    }

    // Returns the total number of collisions from the underlying map
    public long getCollisionCount() {
        return map.getCollisionCount();
    }

    // Estimates the memory usage of the set
    public long estimateMemoryUsage() {
        return map.estimateMemoryUsage();
    }
}