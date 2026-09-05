
#ifndef ANSI_TERMINAL_H
#define ANSI_TERMINAL_H
#include <termios.h>  // For terminal control
#include <string>

class AnsiTerminal {
public:
    static const std::string GREEN;
    static const std::string DEFAULT;

    static void setForegroundColor(const std::string& color);
    static void resetColors();
    
    // Constructor: Sets up the terminal for capturing keystrokes
    AnsiTerminal();

    // Destructor: Restores the terminal settings to the original state
    ~AnsiTerminal();

    // Print text at a specified row and column
    void printAt(int row, int col, const std::string &text);

    // Print text with inverted background at a specified row and column
    void printInvertedAt(int row, int col, const std::string &text);

    // Clear the terminal screen
    void clearScreen();

    // Get a single keystroke from the terminal
    char getKeystroke();

    // Get the arrow key or special key input ('U', 'D', 'L', 'R' for Up, Down, Left, Right),
    // or detect other key combinations such as Alt+Key, Ctrl+Key, etc.
    char getSpecialKey();
    // Yeni metot bildirimi
    void printColoredAt(int row, int col, const std::string &text, const std::string &colorCode);

private:
    struct termios original_tio; // Holds the original terminal settings
};

#endif // ANSI_TERMINAL_H
