#include "IntValueCell.h"
#include <stdexcept>
#include <utility>
#include <string>
#include "Spreadsheet.h"


void IntValueCell::setContent(const std::string& newContent, Spreadsheet& spreadsheet) {
    content = newContent;
    try {
        value = std::stoi(newContent);
    } catch (const std::exception&) {
        value = 0;
    }
}

std::string IntValueCell::getContent() const {
    return content;
}

void IntValueCell::setValue(double newValue) {
    value = static_cast<int>(newValue);
    content = std::to_string(value); // İçeriği güncelle
}

double IntValueCell::getValue() const {
    return value;
}

bool IntValueCell::isFormula() const {
    return false;
}

void IntValueCell::addDependent(int row, int col) {
    dependents.emplace_back(row, col);
}

std::vector<std::pair<int, int>> IntValueCell::getDependents() const {
    return dependents;
}

void IntValueCell::clearDependents() {
    dependents.clear();
}

