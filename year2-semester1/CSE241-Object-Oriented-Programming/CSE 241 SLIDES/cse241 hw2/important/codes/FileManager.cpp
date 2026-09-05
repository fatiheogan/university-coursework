#include "FileManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

void FileManager::saveToCSV(const Spreadsheet& sheet, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << filename << "\n";
        return;
    }

    for (int row = 0; row < 5; ++row) { // 5 yerine sheet'in satır sayısı kullanılabilir
        for (int col = 0; col < 5; ++col) { // 5 yerine sütun sayısı
            file << sheet.getCellContent(row, col);
            if (col < 4) file << ","; // Son sütundan sonra virgül koyma
        }
        file << "\n";
    }

    file.close();
    std::cout << "Spreadsheet saved to " << filename << "\n";
}

void FileManager::loadFromCSV(Spreadsheet& sheet, const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for reading: " << filename << "\n";
        return;
    }

    std::string line;
    int row = 0;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        int col = 0;

        while (std::getline(ss, cell, ',')) {
            sheet.setCellContent(row, col, cell);
            ++col;
        }
        ++row;
    }

    file.close();
    std::cout << "Spreadsheet loaded from " << filename << "\n";
}

