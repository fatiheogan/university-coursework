#ifndef STRINGVALUECELL_H
#define STRINGVALUECELL_H

#include "Cell.h"
#include <string>
#include <vector>
#include <utility>

class StringValueCell : public Cell {
private:
    std::string value;
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

#endif // STRINGVALUECELL_H

