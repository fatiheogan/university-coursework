public class GTUHashMap<K, V> {
    private Entry<K, V>[] table;  // Array to store the key-value pairs
    private int size;            // Number of active entries in the map
    private int capacity;        // Total slots available in the array
    private static final double LOAD_FACTOR = 0.75;  // Maximum fill ratio before resizing
    private long collisionCount;  // Tracks the number of collisions

    // Constructor: Creates a new hash map with initial capacity of 16
    @SuppressWarnings("unchecked")
    public GTUHashMap() {
        this.capacity = 17;  // Start with a prime number for better distribution
        this.table = new Entry[capacity];
        this.size = 0;
        this.collisionCount = 0;
    }

    // Calculates the hash value for a key
    private int hash(K key) {
        return Math.abs(key.hashCode() % capacity);
    }

    // Finds the slot for a key using quadratic probing
    private int findSlot(K key, boolean searchForEmpty) {
        int index = hash(key);  // Start with the hashed index
        int originalIndex = index;
        int i = 0;  // Attempt counter for quadratic probing

        do {
            if (table[index] == null) {  // If slot is empty
                return searchForEmpty ? index : -1;
            } else if (table[index].isDeleted && searchForEmpty) {  // If slot is marked deleted
                return index;
            } else if (!table[index].isDeleted && table[index].key.equals(key)) {  // If key is found
                return index;
            }
            collisionCount++;  // Increment collision count
            i++;  // Increment attempt counter
            index = (hash(key) + i * i) % capacity;  // Quadratic probing: index + i^2
        } while (index != originalIndex && i < capacity);  // Continue until back to start or max attempts

        return searchForEmpty ? index : -1;  // Return index or -1 if not found
    }

    // Adds or updates a key-value pair in the map
    public void put(K key, V value) {
        if (key == null) {  // Check if key is null
            throw new IllegalArgumentException("Key cannot be null");
        }

        if ((size + 1) > capacity * LOAD_FACTOR) {  // Check if resize is needed
            resize();
        }

        int index = findSlot(key, true);  // Find a slot
        if (index == -1) {  // If no slot available
            return;
        }

        if (table[index] == null || table[index].isDeleted) {  // If slot is empty or deleted
            table[index] = new Entry<>(key, value);  // Add new entry
            size++;  // Increase size
        } else {  // If slot has an entry
            table[index].value = value;  // Update the value
        }
    }

    // Gets the value for a given key
    public V get(K key) {
        if (key == null) {  // Check if key is null
            return null;
        }

        int index = findSlot(key, false);  // Find the key
        if (index != -1 && table[index] != null && !table[index].isDeleted) {  // If key is found
            return table[index].value;  // Return the value
        }
        return null;  // Return null if not found
    }

    // Marks a key-value pair as deleted
    public void remove(K key) {
        if (key == null) {  // Check if key is null
            return;
        }

        int index = findSlot(key, false);  // Find the key
        if (index != -1 && table[index] != null && !table[index].isDeleted) {  // If key is found
            table[index].isDeleted = true;  // Mark as deleted
            size--;  // Decrease size
        }
    }

    // Checks if the map contains a given key
    public boolean containsKey(K key) {
        if (key == null) {  // Check if key is null
            return false;
        }

        int index = findSlot(key, false);  // Find the key
        return index != -1 && table[index] != null && !table[index].isDeleted;  // Return true if found
    }

    // Returns the number of active entries
    public int size() {
        return size;
    }

    // Finds the next prime number for capacity
    private int nextPrime(int n) {
        while (!isPrime(n)) {
            n++;
        }
        return n;
    }

    // Checks if a number is prime
    private boolean isPrime(int n) {
        if (n <= 1) return false;
        for (int i = 2; i <= Math.sqrt(n); i++) {
            if (n % i == 0) return false;
        }
        return true;
    }

    // Resizes the map when it gets too full
    @SuppressWarnings("unchecked")
    private void resize() {
        Entry<K, V>[] oldTable = table;  // Save the old table
        capacity = nextPrime(capacity * 2);  // Double and find next prime
        table = new Entry[capacity];  // Cast from Object[] to Entry<K, V>[], needs @SuppressWarnings
        size = 0;  // Reset size
        collisionCount = 0;  // Reset collision count after resize

        for (Entry<K, V> entry : oldTable) {  // Copy old entries to new table
            if (entry != null && !entry.isDeleted) {  // If entry is valid
                put(entry.key, entry.value);  // Add entry to new table
            }
        }
    }

    // Returns the total number of collisions
    public long getCollisionCount() {
        return collisionCount;
    }

    // Estimates the memory usage in bytes
    public long estimateMemoryUsage() {
        // Approximate memory usage:
        // - Reference to table: 8 bytes
        // - Each Entry object: ~32 bytes (key, value, isDeleted, references)
        // - Array overhead: ~16 bytes
        long arrayMemory = 16 + capacity * 8;  // Array overhead + references
        long entriesMemory = 0;
        for (Entry<K, V> entry : table) {
            if (entry != null && !entry.isDeleted) {
                entriesMemory += 32;  // Approximate size of each Entry
            }
        }
        return arrayMemory + entriesMemory;
    }
}