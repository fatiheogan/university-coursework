#ifndef FORMULAPARSER_H
#define FORMULAPARSER_H

#include <string>
#include "Spreadsheet.h"

class FormulaParser {
public:
    // Formülü değerlendir ve sonucu döndür
    static double evaluate(const std::string& formula, const Spreadsheet& sheet);
};

#endif // FORMULAPARSER_H

