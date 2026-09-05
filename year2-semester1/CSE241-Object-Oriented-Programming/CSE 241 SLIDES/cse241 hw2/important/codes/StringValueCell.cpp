#include "StringValueCell.h"
#include <stdexcept>
#include <utility>
#include <string>

void StringValueCell::setContent(const std::string& newContent, Spreadsheet& spreadsheet) {
    content = newContent;
    value = newContent; // String hücrelerde içerik direkt olarak saklanır.
}


std::string StringValueCell::getContent() const {
    return content;
}

void StringValueCell::setValue(double newValue) {
    throw std::logic_error("StringValueCell cannot store a numeric value.");
}

double StringValueCell::getValue() const {
    throw std::logic_error("StringValueCell does not have a numeric value.");
}

bool StringValueCell::isFormula() const {
    return false;
}

void StringValueCell::addDependent(int row, int col) {
    dependents.emplace_back(row, col);
}

std::vector<std::pair<int, int>> StringValueCell::getDependents() const {
    return dependents;
}

void StringValueCell::clearDependents() {
    dependents.clear();
}

