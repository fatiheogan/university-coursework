#ifndef CELL_H
#define CELL_H

#include <string>
#include <vector>
#include <utility>

class Spreadsheet; // Forward declaration

class Cell {
public:
    virtual ~Cell() = default;

    virtual void setContent(const std::string& newContent, Spreadsheet& spreadsheet) = 0; // Yeni imza
    virtual std::string getContent() const = 0;

    virtual void setValue(double newValue) = 0;
    virtual double getValue() const = 0;

    virtual bool isFormula() const = 0;

    virtual void addDependent(int row, int col) = 0;
    virtual std::vector<std::pair<int, int>> getDependents() const = 0;
    virtual void clearDependents() = 0;
};

#endif // CELL_H

