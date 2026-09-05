#include "DoubleValueCell.h"
#include <stdexcept>
#include <utility>
#include <string>

void DoubleValueCell::setContent(const std::string& newContent, Spreadsheet& spreadsheet) {
    content = newContent;
    try {
        value = std::stod(newContent);
    } catch (const std::exception&) {
        value = 0.0;
    }
}

std::string DoubleValueCell::getContent() const {
    return content;
}

void DoubleValueCell::setValue(double newValue) {
    value = newValue;
    content = std::to_string(value); // İçeriği güncelle
}

double DoubleValueCell::getValue() const {
    return value;
}

bool DoubleValueCell::isFormula() const {
    return false;
}

void DoubleValueCell::addDependent(int row, int col) {
    dependents.emplace_back(row, col);
}

std::vector<std::pair<int, int>> DoubleValueCell::getDependents() const {
    return dependents;
}

void DoubleValueCell::clearDependents() {
    dependents.clear();
}

