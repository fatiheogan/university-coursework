#include "Spreadsheet.h"
#include "FormulaParser.h"
#include "AnsiTerminal.h"
#include "IntValueCell.h"
#include "StringValueCell.h"
#include "DoubleValueCell.h"
#include "FormulaCell.h"
#include <stdexcept>
#include <iomanip>
#include <iostream>

Spreadsheet::Spreadsheet(int rowCount, int colCount)
    : rows(rowCount), cols(colCount) {
    grid.resize(rowCount);
    for (int row = 0; row < rowCount; ++row) {
        grid[row].resize(colCount);
        for (int col = 0; col < colCount; ++col) {
            grid[row][col] = std::make_unique<StringValueCell>();
        }
    }
}


std::unique_ptr<Cell> Spreadsheet::createCell(const std::string& content) {
    if (content.empty()) {
        return std::make_unique<StringValueCell>();
    } else if (content[0] == '=') {
        return std::make_unique<FormulaCell>();
    } else {
        try {
            if (content.find('.') != std::string::npos) {
                std::stod(content); // Test if it's a double
                return std::make_unique<DoubleValueCell>();
            } else {
                std::stoi(content); // Test if it's an integer
                return std::make_unique<IntValueCell>();
            }
        } catch (const std::exception&) {
            return std::make_unique<StringValueCell>();
        }
    }
}

void Spreadsheet::setCellContent(int row, int col, const std::string& content) {

    if (row >= 0 && row < rows && col >= 0 && col < cols) {
        grid[row][col] = createCell(content);

        grid[row][col]->clearDependents();

        grid[row][col]->setContent(content, *this);

        if (content[0] == '=') {
            auto dependencies = parseFormulaDependencies(content);
            for (const auto& [depRow, depCol] : dependencies) {
                if (depRow >= 0 && depRow < rows && depCol >= 0 && depCol < cols) {
                    grid[depRow][depCol]->addDependent(row, col);
                }
            }
        }

        updateDependents(row, col);
    } else {
        throw std::out_of_range("Cell position is out of bounds.");
    }
}





std::string Spreadsheet::getCellContent(int row, int col) const {
    if (row >= 0 && row < rows && col >= 0 && col < cols) {
        return grid[row][col]->getContent();
    }
    return "";
}

void Spreadsheet::updateDependents(int row, int col) {
    const auto& dependents = grid[row][col]->getDependents();
    for (const auto& [depRow, depCol] : dependents) {
        if (grid[depRow][depCol]->isFormula()) {
            try {
                double result = FormulaParser::evaluate(grid[depRow][depCol]->getContent(), *this);
                grid[depRow][depCol]->setValue(result);
            } catch (const std::exception& e) {
                std::cerr << "Error updating dependent cell: " << e.what() << "\n";
                grid[depRow][depCol]->setContent("ERROR", *this); // Yeni imzaya uygun hale getirildi
            }
            updateDependents(depRow, depCol); // Rekürsif çağrı
        }
    }
}


std::vector<std::pair<int, int>> Spreadsheet::parseFormulaDependencies(const std::string& formula) {
    std::vector<std::pair<int, int>> dependencies;
    if (formula.empty() || formula[0] != '=') return dependencies;

    std::string expression = formula.substr(1); // Remove '='
    std::string token;

    for (size_t i = 0; i <= expression.size(); ++i) {
        if (i < expression.size() && (std::isalnum(expression[i]) || expression[i] == '.')) {
            token += expression[i];
        } else {
            if (!token.empty() && std::isalpha(token[0])) {
                int col = token[0] - 'A';
                int row = std::stoi(token.substr(1)) - 1;
                dependencies.emplace_back(row, col);
            }
            token.clear();
        }
    }
    return dependencies;
}

void Spreadsheet::printSpreadsheet(AnsiTerminal& terminal) const {
    terminal.clearScreen();
    std::cout << "   ";
    for (int col = 0; col < cols; ++col) {
        std::cout << std::setw(5) << char('A' + col) << " ";
    }
    std::cout << "\n";

    for (int row = 0; row < rows; ++row) {
        std::cout << std::setw(2) << row + 1 << " ";
        for (int col = 0; col < cols; ++col) {
            if (row == selectedRow && col == selectedCol) {
                if (grid[row][col]->getContent().empty()) {
                    // Hücre boşsa * işaretini göster
                    std::cout << std::setw(5) << "*" << " ";
                } else {
                    // Hücre doluysa yeşil renkte içeriği göster
                    AnsiTerminal::setForegroundColor(AnsiTerminal::GREEN);
                    std::cout << std::setw(5) << grid[row][col]->getContent() << " ";
                    AnsiTerminal::resetColors();
                }
            } else {
                // Normal hücre içeriğini göster
                std::cout << std::setw(5) << grid[row][col]->getContent() << " ";
            }
        }
        std::cout << "\n";
    }
}


void Spreadsheet::moveSelection(char direction) {
    switch (direction) {
        case 'U': if (selectedRow > 0) --selectedRow; break;
        case 'D': if (selectedRow < rows - 1) ++selectedRow; break;
        case 'L': if (selectedCol > 0) --selectedCol; break;
        case 'R': if (selectedCol < cols - 1) ++selectedCol; break;
    }
}

double Spreadsheet::getCellValue(int row, int col) const {
    if (row >= 0 && row < rows && col >= 0 && col < cols) {
        return grid[row][col]->getValue();
    }
    throw std::out_of_range("Cell out of range.");
}

