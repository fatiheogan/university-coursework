// Updated Spreadsheet.h
#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include "AnsiTerminal.h"
#include "Cell.h"
#include <vector>
#include <memory>
#include <string>

class Spreadsheet {
private:
    std::vector<std::vector<std::unique_ptr<Cell>>> grid; // 2D hücre tablosu
    int rows; // Satır sayısı
    int cols; // Sütun sayısı
    int selectedRow = 0; // Varsayılan olarak ilk satır
    int selectedCol = 0; // Varsayılan olarak ilk sütun

    void updateDependents(int row, int col); // Bağımlı hücreleri yeniden hesapla
    std::vector<std::pair<int, int>> parseFormulaDependencies(const std::string& formula); // Formülden bağımlılıkları çözümle

    // Yeni metot: Hücre türünü belirle ve oluştur
    std::unique_ptr<Cell> createCell(const std::string& content);

public:
    int getSelectedRow() const { return selectedRow; }
    int getSelectedCol() const { return selectedCol; }
    void moveSelection(char direction); // Yön hareket metodu
    double getCellValue(int row, int col) const;

    Spreadsheet(int rowCount, int colCount); // Yapıcı

    void setCellContent(int row, int col, const std::string& content); // Hücre içeriğini ayarla
    std::string getCellContent(int row, int col) const;               // Hücre içeriğini al

    void printSpreadsheet(AnsiTerminal& terminal) const;
};

#endif // SPREADSHEET_H

