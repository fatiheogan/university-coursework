#include "Table.h"  // Table sınıfının başlık dosyasını dahil eder
#include <cmath>     // Matematiksel işlemler için gerekli 
#include <sstream>   // String akışları (ostringstream, istringstream) için gerekli başlık dosyası
#include <iostream>  
#include <stdexcept> // Hata işleme (exception handling) için gerekli başlık dosyası 

// Constructor: Satır ve sütun sayısına göre tabloyu başlatır
Table::Table(int rows, int cols) : rowCount(rows), colCount(cols) {
    grid.resize(rowCount, std::vector<Cell>(colCount)); // Tabloyu boyutlara göre yeniden boyutlandırır
}

// Formül girdisini temizler (şu an için sadece kopyalar, gelecekte eklenebilir)
std::string Table::cleanFormulaInput(const std::string& input) {
    std::string cleaned = input;
    return cleaned; // Girdiği olduğu gibi döner
}

// Sütun harfini, sütun indeksine dönüştürür (A -> 0, B -> 1, ...)
int Table::columnToIndex(const std::string& col) const {
    return col[0] - 'A'; // Harften sayıya dönüşüm
}

// Tabloyu günceller ve formülleri hesaplar
void Table::updateFormulas() {
    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < colCount; ++col) {
            if (grid[row][col].isFormulaCell()) { // Hücre formül içeriyorsa
                std::string formula = grid[row][col].getFormula();
                std::string cleanFormula = cleanFormulaInput(formula); // Formülü temizle
                double result = evaluateFormula(cleanFormula, row, col); // Formülü hesapla
                grid[row][col].setValue(std::to_string(result)); // Hesaplanan sonucu hücreye ata
            }
        }
    }
}

// Formülü değerlendirir ve sonucu döner
// powered by ChatGPT
double Table::evaluateFormula(const std::string& formula, int currentRow, int currentCol) {
    std::istringstream iss(formula);
    double lhs, rhs; // Sol ve sağ operandlar
    char op; // İşlem operatörü

    iss >> lhs; // İlk sayıyı al
    while (iss >> op >> rhs) { // İşlem sırasını uygula
        switch (op) {
            case '+': lhs += rhs; break; // Toplama
            case '-': lhs -= rhs; break; // Çıkarma
            case '*': lhs *= rhs; break; // Çarpma
            case '/':
                if (rhs == 0) throw std::invalid_argument("Cant divide by zero"); // Bölme sıfıra yapılırsa hata
                lhs /= rhs; // Bölme
                break;
            default: throw std::invalid_argument("Invalid operator"); // Geçersiz operatör
        }
    }

    return lhs; // Sonucu döner
} // powered by ChatGPT

// Tabloyu ekrana yazdırır
void Table::display() const {
    for (int col = 0; col < colCount; ++col) {
        std::cout << static_cast<char>('A' + col) << "\t"; // Sütun başlıklarını yazdır
    }
    std::cout << std::endl;

    for (int row = 0; row < rowCount; ++row) {
        std::cout << row + 1 << "\t"; // Satır numarasını yazdır
        for (int col = 0; col < colCount; ++col) {
            std::cout << getCell(row, col) << "\t"; // Hücredeki değeri yazdır
        }
        std::cout << std::endl;
    }
}

// Verilen hücreye ait değeri döner
std::string Table::getCell(int row, int col) const {
    return grid[row][col].getValue(); // Hücre değerini döner
}

// Hücre referansının geçerli olup olmadığını kontrol eder
bool Table::isValidCell(const std::string& reference) const {
    auto [row, col] = parseCellReference(reference); // Referansı çözümle
    return (row >= 0 && row < rowCount && col >= 0 && col < colCount); // Geçerli satır ve sütun olup olmadığını kontrol et
}

// Hücre referansına karşılık gelen değeri döner
std::string Table::getCellByReference(const std::string& reference) const {
    auto [row, col] = parseCellReference(reference); // Referansı çözümle
    return grid[row][col].getValue(); // Hücre değerini döner
}

// Hücre referansını satır ve sütun indekslerine dönüştürür
// powered by ChatGPT
std::pair<int, int> Table::parseCellReference(const std::string& cellRef) const {
    int col = columnToIndex(cellRef.substr(0, 1)); // Sütun harfini indeks olarak dönüştür
    int row = std::stoi(cellRef.substr(1)) - 1; // Satır numarasını indeks olarak dönüştür
    return {row, col}; // Satır ve sütun indeksini döner
} // powered by ChatGPT

