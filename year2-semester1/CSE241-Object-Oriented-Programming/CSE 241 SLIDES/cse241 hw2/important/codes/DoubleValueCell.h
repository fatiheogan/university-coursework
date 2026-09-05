#ifndef DOUBLEVALUECELL_H
#define DOUBLEVALUECELL_H

#include "Cell.h"
#include <string>
#include <vector>
#include <utility>
#include "Spreadsheet.h"

class DoubleValueCell : public Cell {
private:
    double value;
    std::string content;
    std::vector<std::pair<int, int>> dependents;

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

#endif // DOUBLEVALUECELL_H

