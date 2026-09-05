#ifndef ANSI_TERMINAL_H
#define ANSI_TERMINAL_H
#include <termios.h>  // Terminal kontrolü için
#include <string>

class AnsiTerminal {
public:
    // Constructor: Tuş girişlerini yakalamak için terminali ayarlar
    AnsiTerminal();

    // Destructor: Terminal ayarlarını orijinal durumuna geri döndürür
    ~AnsiTerminal();

    // Belirli bir satır ve sütuna metin yazdırır
    void printAt(int row, int col, const std::string &text);

    // Belirli bir satır ve sütuna ters arka planla (inverted) metin yazdırır
    void printInvertedAt(int row, int col, const std::string &text);

    // Terminal ekranını temizler
    void clearScreen();

    // Terminalden tek bir tuş okur
    char getKeystroke();

    // Ok tuşu ya da özel tuş girişini alır ('U', 'D', 'L', 'R' yukarı, aşağı, sol, sağ için),
    // veya Alt+Tuş, Ctrl+Tuş gibi diğer tuş kombinasyonlarını algılar
    char getSpecialKey();

private:
    struct termios original_tio; // Orijinal terminal ayarlarını saklar
};

#endif // ANSI_TERMINAL_H

