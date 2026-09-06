public class EditDistanceHelper {
    private static final String ALPHABET = "abcdefghijklmnopqrstuvwxyz";  // All lowercase letters for generating variants

    // Generates a list of suggested words with edit distance 1 or 2 from the input word
    public static CustomList<String> generateEditDistance(String word, GTUHashSet<String> dictionary) {
        CustomList<String> suggestions = new CustomList<>();  // List to store suggested words

        // Generate variants with edit distance 1
        for (String variant : generateEditDistance1(word)) {
            if (dictionary.contains(variant)) {  // Check if variant is a valid word
                suggestions.add(variant);  // Add valid word to suggestions
            }
        }

        // Generate variants with edit distance 2 (one edit on top of edit distance 1)
        for (String variant1 : generateEditDistance1(word)) {
            for (String variant2 : generateEditDistance1(variant1)) {
                if (dictionary.contains(variant2)) {  // Check if variant is a valid word
                    suggestions.add(variant2);  // Add valid word to suggestions
                }
            }
        }
        return suggestions;  // Return the list of suggestions
    }

    // Generates all possible variants of a word with edit distance 1
    private static CustomList<String> generateEditDistance1(String word) {
        CustomList<String> variants = new CustomList<>();  // List to store variants

        // Deletion: Remove each character one by one
        for (int i = 0; i < word.length(); i++) {
            String variant = word.substring(0, i) + word.substring(i + 1);  // Remove character at index i
            variants.add(variant);  // Add the new variant
        }

        // Insertion: Add each letter from alphabet at every position
        for (int i = 0; i <= word.length(); i++) {
            for (char c : ALPHABET.toCharArray()) {
                String variant = word.substring(0, i) + c + word.substring(i);  // Insert character c at position i
                variants.add(variant);  // Add the new variant
            }
        }

        // Substitution: Replace each character with every other letter
        for (int i = 0; i < word.length(); i++) {
            for (char c : ALPHABET.toCharArray()) {
                if (c != word.charAt(i)) {  // Skip if the letter is the same as the original
                    String variant = word.substring(0, i) + c + word.substring(i + 1);  // Replace character at i with c
                    variants.add(variant);  // Add the new variant
                }
            }
        }

        return variants;  // Return the list of variants
    }
}