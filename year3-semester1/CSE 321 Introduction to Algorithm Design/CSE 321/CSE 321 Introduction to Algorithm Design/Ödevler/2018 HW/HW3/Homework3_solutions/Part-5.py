def ReturnPattern(string, dictionary, newDictionary={}, pattern="", patternVal='A', count=1):
    string = string.lower()

    if (len(string) == 0):
        return pattern

    subStr = string[0: count]

    if (len(string) < count):
        pattern += str(patternVal)
        return pattern

    if subStr in dictionary:
        if subStr in newDictionary:
            pattern += str(newDictionary[subStr])
            return ReturnPattern(string[count:], dictionary, newDictionary, pattern, patternVal, 1)
        else:
            newDictionary[subStr] = patternVal
            pattern += str(patternVal)
            return ReturnPattern(string[count:], dictionary, newDictionary, pattern, chr(ord(patternVal) + 1), 1)
    else:
        return ReturnPattern(string, dictionary, newDictionary, pattern, patternVal, count + 1)

if __name__ == "__main__":
    string = "thestringisvalidstring"
    dictionary = {"the","string","valid","is"}
    pattern = "ABCDB"
    extracted_pattern=ReturnPattern(string, dictionary)
    if extracted_pattern==pattern:
        print("Generated pattern =",extracted_pattern,"; so pattern is valid on the given string")
    else:
        print("Generated pattern =", extracted_pattern, "; so pattern is not valid on the given string")

# Algorithm Analysis
# Let's say longest word in the dictionary consist of m letter.
# The algorithm checks is there a successfully matched word
# at each position of the test string.
# Therefore the algorithm iterates n (size of input text) x m times at worst case.
# Thus, time complexity of the algorithm is O(nm) at the worst case.
