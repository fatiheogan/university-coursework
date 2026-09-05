#ifndef PLACE_H
#define PLACE_H

#include "Table.h"
#include "AnsiTerminal.h"

// Place sınıfı, kullanıcıyı tablo üzerinde yönlendiren sınıftır
class Place {
private:
    Table& table; // Tabloyu tutacak referans
    AnsiTerminal& terminal; // Terminali tutacak referans
    int currentRow; // Mevcut satır numarası
    int currentCol; // Mevcut sütun numarası

public:
    // Constructor: Table ve AnsiTerminal nesneleri ile başlangıç değerlerini ayarlayan fonksiyon
    Place(Table& sheet, AnsiTerminal& term);

    // Kullanıcıyı tablo üzerinde yönlendiren fonksiyon
    void navigate();
};

#endif // PLACE_H

