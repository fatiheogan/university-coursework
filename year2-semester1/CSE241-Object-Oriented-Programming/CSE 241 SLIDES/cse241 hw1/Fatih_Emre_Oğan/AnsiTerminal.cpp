#include "AnsiTerminal.h"
#include <iostream>
#include <unistd.h>   // read() fonksiyonu için
#include <termios.h>  // Terminal kontrolü için

// Constructor: Terminali non-canonical (satır-bazlı olmayan) moda ayarla
AnsiTerminal::AnsiTerminal() {
    // Orijinal terminal ayarlarını kaydet
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;

    // Canonical modu ve echo'yu devre dışı bırak (anlık giriş okumak için)
    new_tio.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

// Destructor: Terminal ayarlarını eski haline döndür
AnsiTerminal::~AnsiTerminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

// Belirtilen konuma metin yazdırır
void AnsiTerminal::printAt(int row, int col, const std::string &text) {
    // ANSI kaçış kodları ile konum belirle ve metni yazdır
    std::cout << "\033[" << row << ";" << col << "H" << text << std::flush;
}

// Belirtilen konuma ters arka planla (inverted) metin yazdırır
void AnsiTerminal::printInvertedAt(int row, int col, const std::string &text) {
    // ANSI kaçış kodlarıyla ters arka plan modunu etkinleştir (\033[7m) ve ardından sıfırla (\033[0m)
    std::cout << "\033[" << row << ";" << col << "H\033[7m" << text << "\033[0m" << std::flush;
}

// Terminal ekranını temizler
void AnsiTerminal::clearScreen() {
    // Ekranı temizle (\033[2J) ve imleci başlangıç pozisyonuna taşı (\033[H)
    std::cout << "\033[2J\033[H" << std::flush;
}

// Terminalden tek bir tuş okur
char AnsiTerminal::getKeystroke() {
    char ch;
    read(STDIN_FILENO, &ch, 1);  // İlk karakteri oku

    // Eğer karakter kontrol karakterleri aralığındaysa (ASCII 0x01 - 0x1A)
    // Ctrl+A ile Ctrl+Z, 'a' ile '@' arasında eşlenir
    if (ch >= 1 && ch <= 26) {
        // Ctrl+A ile Ctrl+Z'yi daha okunabilir bir formata eşle
        return ch + 'A' - 1;
    }

    // Eğer normal bir karakterse olduğu gibi döndür
    return ch;
}

// Ok tuşları, Alt tuşları ve diğer özel tuşları işler
char AnsiTerminal::getSpecialKey() {
    char ch = getKeystroke();

    // Eğer karakter bir escape karakteriyse (\033), bir kaçış dizisi ile karşılaşabiliriz
    if (ch == '\033') {
        // Bir sonraki karakteri kontrol ederek ok tuşu veya Alt kombinasyonu olup olmadığını kontrol et
        char next_ch;
        if (read(STDIN_FILENO, &next_ch, 1) == 0) return '\033';  // Eğer başka giriş yoksa, sadece ESC döndür

        // Ok tuşları ve bazı fonksiyon tuşları '\033' ardından '[' ile başlayan diziler üretir
        if (next_ch == '[') {
            char arrow_key;
            if (read(STDIN_FILENO, &arrow_key, 1) == 1) {
                // Ok tuşlarına göre eşleme yap
                switch (arrow_key) {
                    case 'A': return 'U'; // Yukarı ok
                    case 'B': return 'D'; // Aşağı ok
                    case 'C': return 'R'; // Sağ ok
                    case 'D': return 'L'; // Sol ok
                    // Home, End, PgUp, PgDn gibi tuşlar için ek durumlar eklenebilir
                }
            }
        } else {
            // Eğer bir ok dizisi değilse, Alt+Tuş kombinasyonu olabilir
            // İkinci karakteri, Alt ile birlikte basılan tuş olarak döndür
            return next_ch | 0x80;  // Yüksek biti ayarla (Alt'ı ayırt etmek için)
        }
    }

    // Eğer normal bir karakter veya Ctrl kombinasyonuysa olduğu gibi döndür
    return ch;
}

