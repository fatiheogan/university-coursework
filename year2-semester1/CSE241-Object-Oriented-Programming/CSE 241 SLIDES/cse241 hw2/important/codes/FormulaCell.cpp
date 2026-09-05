#include "FormulaCell.h"
#include "Spreadsheet.h"
#include <stdexcept>
#include <sstream>
#include <cctype>
#include <iostream>

// Formülü değerlendirir
double FormulaCell::evaluateFormula(const std::string& formula, const Spreadsheet& spreadsheet) {
    return FormulaParser::evaluate(formula, spreadsheet);
}



void FormulaCell::setContent(const std::string& newContent, Spreadsheet& spreadsheet) {
    content = newContent;
    if (content[0] == '=') {
        value = evaluateFormula(content, spreadsheet);
    } else {
        throw std::logic_error("FormulaCell can only accept formulas.");
    }
}



std::string FormulaCell::getContent() const {
    // Değer string olarak döndürülür
    return std::to_string(value);
}


void FormulaCell::setValue(double newValue) {
    throw std::logic_error("FormulaCell value is determined by its formula.");
}

double FormulaCell::getValue() const {
    return value;
}

bool FormulaCell::isFormula() const {
    return true;
}

void FormulaCell::addDependent(int row, int col) {
    dependents.emplace_back(row, col);
}

std::vector<std::pair<int, int>> FormulaCell::getDependents() const {
    return dependents;
}

void FormulaCell::clearDependents() {
    dependents.clear();
}

