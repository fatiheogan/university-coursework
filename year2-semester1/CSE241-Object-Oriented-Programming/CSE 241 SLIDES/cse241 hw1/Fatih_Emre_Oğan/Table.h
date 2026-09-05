#ifndef TABLE_H
#define TABLE_H

#include <vector>    // Dinamik dizi (vector) kullanabilmek için gerekli başlık dosyası
#include <string>    // String sınıfı için gerekli başlık dosyası
#include <iostream>  // Giriş/çıkış işlemleri için gerekli başlık dosyası (örn: std::cout, std::cin)
#include "Cell.h"    // Hücre sınıfı başlık dosyasını dahil eder
#include <map>       // Harita (map) veri yapısını kullanabilmek için gerekli başlık dosyası

// Table sınıfı, hücrelerden oluşan bir tabloyu temsil eder
class Table {
private:
    std::vector<std::vector<Cell>> grid; // Hücrelerden oluşan 2D tablo (satır ve sütunlar)
    int rowCount; // Tablo satır sayısı
    int colCount; // Tablo sütun sayısı

    // Hücre referansını çözümleyerek satır ve sütun indeksini döner
    // powered by ChatGPT
    std::pair<int, int> parseCellReference(const std::string& cellRef) const;

    // Formül girdisini temizler (şu an için sadece kopyalar, gelecekte düzenleme yapılabilir)
    std::string cleanFormulaInput(const std::string& input);

    // Sütun harfini, sütun indeksine dönüştürür (A -> 0, B -> 1, ...)
    int columnToIndex(const std::string& col) const;

public:
    // Constructor: Tabloyu satır ve sütun sayısı ile başlatır
    Table(int rows, int cols);

    // Formülleri günceller (tablodaki formül hücrelerini değerlendirir)
    void updateFormulas();

    // Satır sayısını döner
    int getRowCount() const { return rowCount; }

    // Sütun sayısını döner
    int getColCount() const { return colCount; }

    // Verilen hücrenin formülünü döner
    std::string getCellFormula(int currRow, int currCol) const { return grid[currRow][currCol].getFormula(); }

    // Hücreye veri ekler
    void setCell(int row, int col, const std::string& value);

    // Hücredeki değeri döner
    std::string getCell(int row, int col) const;

    // Hücre referansına karşılık gelen değeri döner (örneğin: "A1")
    std::string getCellByReference(const std::string& reference) const;

    // Formülü değerlendirir ve sonucu döner
    // powered by ChatGPT
    double evaluateFormula(const std::string& formula, int currentRow, int currentCol);

    // Tabloyu ekrana yazdırır
    void display() const;

    // Hücrenin geçerli olup olmadığını kontrol eder
    bool isValidCell(const std::string& reference) const;
};

#endif // TABLE_H

