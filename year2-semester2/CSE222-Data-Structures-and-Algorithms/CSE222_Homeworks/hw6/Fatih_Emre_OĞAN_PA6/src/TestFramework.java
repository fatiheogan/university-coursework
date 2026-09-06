public class TestFramework {
    private int passedTests = 0;
    private int failedTests = 0;

    // Checks if two values are equal
    public <T> void assertEquals(T expected, T actual, String testName) {
        if (expected == null && actual == null || expected != null && expected.equals(actual)) {
            System.out.println("Test Passed: " + testName);
            passedTests++;
        } else {
            System.out.println("Test Failed: " + testName + " | Expected: " + expected + ", Actual: " + actual);
            failedTests++;
        }
    }

    // Checks if a condition is true
    public void assertTrue(boolean condition, String testName) {
        if (condition) {
            System.out.println("Test Passed: " + testName);
            passedTests++;
        } else {
            System.out.println("Test Failed: " + testName + " | Condition is false");
            failedTests++;
        }
    }

    // Checks if a condition is false
    public void assertFalse(boolean condition, String testName) {
        assertTrue(!condition, testName);
    }

    // Prints the summary of test results
    public void printSummary() {
        System.out.println("\nTest Summary:");
        System.out.println("Passed: " + passedTests);
        System.out.println("Failed: " + failedTests);
        System.out.println("Total: " + (passedTests + failedTests));
    }
}