#include "Place.h"
#include <iostream>

// Constructor: Table ve AnsiTerminal nesneleri ile birlikte başlangıç değerlerini ayarlıyor
Place::Place(Table& sheet, AnsiTerminal& term)
    : table(sheet), terminal(term), currentRow(1), currentCol(1) {}

// Kullanıcıyı tablodaki hücrelerde yönlendiren fonksiyon
void Place::navigate() {
    char key; // Kullanıcının tuş girişini tutacak değişken

    // Sonsuz döngü, kullanıcı 'q' tuşuna basana kadar devam eder
    while (true) {
        terminal.clearScreen(); // Ekranı temizle
        table.display(); // Tabloyu ekranda göster
        terminal.printInvertedAt(currentRow, currentCol, "*"); // Şu anki konumu '*' ile işaretle

        key = terminal.getSpecialKey(); // Kullanıcının tuş girişini al

        terminal.printAt(currentRow, currentCol, " "); // Önceki işareti kaldır

        // Kullanıcının tuşuna göre hareket et
        switch (key) {
            case 'U': // Yukarı hareket
                if (currentRow > 1) currentRow--; // İlk satıra ulaşılmadıysa yukarı git
                break;
            case 'D': // Aşağı hareket
                if (currentRow < table.getRowCount()) currentRow++; // Son satıra ulaşılmadıysa aşağı git
                break;
            case 'R': // Sağa hareket
                if (currentCol < table.getColCount()) currentCol++; // Son sütuna ulaşılmadıysa sağa git
                break;
            case 'L': // Sola hareket
                if (currentCol > 1) currentCol--; // İlk sütuna ulaşılmadıysa sola git
                break;
            case 'q': // 'q' tuşuna basıldığında çık
                return;
        }
    }
}

