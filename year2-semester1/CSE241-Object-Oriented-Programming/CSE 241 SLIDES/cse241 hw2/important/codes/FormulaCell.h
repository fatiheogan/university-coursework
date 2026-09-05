#ifndef FORMULACELL_H
#define FORMULACELL_H

#include "Cell.h"
#include "Spreadsheet.h"
#include <string>
#include <vector>
#include <utility>
#include "FormulaParser.h"

class FormulaCell : public Cell {
private:
    double value;
    std::string content;
    std::vector<std::pair<int, int>> dependents;

    // Formül çözümleme fonksiyonu
    double evaluateFormula(const std::string& formula, const Spreadsheet& spreadsheet);

public:
    void setContent(const std::string& newContent, Spreadsheet& spreadsheet) override;
    std::string getContent() const override;
    void setValue(double newValue) override;
    double getValue() const override;
    bool isFormula() const override;

    void addDependent(int row, int col) override;
    std::vector<std::pair<int, int>> getDependents() const override;
    void clearDependents() override;
};

#endif // FORMULACELL_H

