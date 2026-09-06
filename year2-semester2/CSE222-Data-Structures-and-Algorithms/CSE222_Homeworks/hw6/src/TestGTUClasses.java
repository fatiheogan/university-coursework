public class TestGTUClasses {
    public static void main(String[] args) {
        TestFramework test = new TestFramework();

        // Test GTUHashMap
        GTUHashMap<String, Integer> map = new GTUHashMap<>();
        map.put("one", 1);
        map.put("two", 2);
        test.assertEquals(1, map.get("one"), "HashMap get one");
        test.assertEquals(2, map.get("two"), "HashMap get two");
        test.assertEquals(2, map.size(), "HashMap size after put");
        test.assertTrue(map.containsKey("one"), "HashMap containsKey one");
        test.assertFalse(map.containsKey("three"), "HashMap containsKey three");
        map.remove("one");
        test.assertEquals(null, map.get("one"), "HashMap get after remove");
        test.assertEquals(1, map.size(), "HashMap size after remove");
        System.out.println("HashMap collisions: " + map.getCollisionCount());
        System.out.println("HashMap memory usage: " + map.estimateMemoryUsage() + " bytes");

        // Test GTUHashSet
        GTUHashSet<String> set = new GTUHashSet<>();
        set.add("apple");
        set.add("banana");
        test.assertTrue(set.contains("apple"), "HashSet contains apple");
        test.assertFalse(set.contains("orange"), "HashSet contains orange");
        test.assertEquals(2, set.size(), "HashSet size after add");
        set.remove("apple");
        test.assertFalse(set.contains("apple"), "HashSet contains after remove");
        test.assertEquals(1, set.size(), "HashSet size after remove");
        set.add("banana");  // Adding duplicate
        test.assertEquals(1, set.size(), "HashSet size after duplicate add");
        System.out.println("HashSet collisions: " + set.getCollisionCount());
        System.out.println("HashSet memory usage: " + set.estimateMemoryUsage() + " bytes");

        // Print test summary
        test.printSummary();
    }
}