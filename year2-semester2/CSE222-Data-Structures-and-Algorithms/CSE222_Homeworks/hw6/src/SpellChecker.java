import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.Scanner;

// Main class to check spelling and suggest words
public class SpellChecker {
    public static void main(String[] args) throws IOException {
        GTUHashSet<String> dictionary = new GTUHashSet<>();  // Create a set to store dictionary words
        
        // Load the dictionary from a file
        BufferedReader reader = new BufferedReader(new FileReader("dictionary.txt"));
        String word;
        while ((word = reader.readLine()) != null) {  // Read each line from the file
            dictionary.add(word.trim());  // Add the word to the dictionary
        }
        reader.close();  // Close the file

        Scanner scanner = new Scanner(System.in);  // Create a scanner to read user input
        while (true) {  // Loop until user enters an empty word
            System.out.print("Enter a word: ");  // Ask user to enter a word
            String input = scanner.nextLine().trim();  // Read the user's word
            if (input.isEmpty()) {  // If the input is empty
                break;  // Exit the loop
            }

            long startTime = System.nanoTime();  // Start measuring time
            if (dictionary.contains(input)) {  // Check if the word is in the dictionary
                System.out.println("Correct.");  // Word is correct
            } else {
                System.out.println("Incorrect.");  // Word is not in dictionary
                System.out.print("Suggestions: ");  // Show suggestions
                CustomList<String> suggestions = EditDistanceHelper.generateEditDistance(input, dictionary);  // Get suggestions
                System.out.println(suggestions);  // Print suggestions
            }
            long endTime = System.nanoTime();  // Stop measuring time
            System.out.printf("Lookup and suggestion took %.2f ms\n", (endTime - startTime) / 1e6);  // Print time taken
            System.out.println("Total collisions: " + dictionary.getCollisionCount());
            System.out.println("Estimated memory usage: " + dictionary.estimateMemoryUsage() + " bytes");
        }
        scanner.close();  // Close the scanner
    }
}