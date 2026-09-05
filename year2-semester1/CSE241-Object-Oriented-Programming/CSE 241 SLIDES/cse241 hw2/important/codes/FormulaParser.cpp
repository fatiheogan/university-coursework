#include "FormulaParser.h"
#include <cctype>      // For isdigit
#include <stdexcept>   // For std::invalid_argument
#include <string>      // For std::stod
#include <iostream> // Debug için


double FormulaParser::evaluate(const std::string& formula, const Spreadsheet& sheet) {
    if (formula.empty() || formula[0] != '=') {
        throw std::invalid_argument("Invalid formula");
    }

    std::string expression = formula.substr(1); // '=' işaretini kaldır
    double result = 0.0;
    char operation = '+'; // İlk işlem toplama varsayılır
    std::string token;

    for (size_t i = 0; i <= expression.size(); ++i) {
        if (i < expression.size() && (std::isalnum(expression[i]) || expression[i] == '.')) {
            token += expression[i]; // Sayı ya da hücre referansını biriktir
        } else {
            if (!token.empty()) {
                double value = 0.0;

                if (std::isalpha(token[0])) { // Hücre referansı (örneğin, A1)
                    int col = token[0] - 'A';               // Sütun indeksi
                    int row = std::stoi(token.substr(1)) - 1; // Satır indeksi
                    value = sheet.getCellValue(row, col);  // Hücre değerini al
                } else { // Sayı çözümlemesi
                    value = std::stod(token);
                }

                // İşlemi uygula
                if (operation == '+') result += value;
                else if (operation == '-') result -= value;
                else if (operation == '*') result *= value;
                else if (operation == '/') {
                    if (value == 0.0) {
                        throw std::logic_error("Division by zero error");
                    }
                    result /= value;
                }

                token.clear(); // Token sıfırla
            }

            // Yeni işlem karakterini oku
            if (i < expression.size()) {
                operation = expression[i];
            }
        }
    }

    return result;
}




