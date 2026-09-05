#include "AnsiTerminal.h"
#include "Table.h"
#include "Place.h"
#include <iostream>

int main() {
    AnsiTerminal terminal; // AnsiTerminal nesnesi oluşturuluyor
    Table table(10, 10);  // 10x10 boyutunda bir tablo oluşturuluyor
    Place place(table, terminal); // Place nesnesi oluşturuluyor, tablo ve terminal parametre olarak alınıyor

    terminal.clearScreen(); // Ekranı temizle
    table.display(); // Tabloyu ekranda göster
    terminal.printInvertedAt(1, 1, "*"); // (1, 1) koordinatına ters renkli '*' karakteri yazdır

    place.navigate(); // Kullanıcıyı yönlendirmek için 'navigate' fonksiyonu çağrılıyor

    terminal.clearScreen(); // İşlem tamamlandıktan sonra ekran temizleniyor
    return 0; // Program başarıyla sonlandırılıyor
}

