public class Entry<K, V> {
    public K key;           // The key part of the entry (can be any type)
    public V value;         // The value part of the entry (can be any type)
    public boolean isDeleted;  // Flag to mark if the entry is deleted

    // Constructor: Creates a new entry with a key and value
    public Entry(K key, V value) {
        this.key = key;      // Set the key
        this.value = value;  // Set the value
        this.isDeleted = false;  // Set deleted flag to false
    }
}