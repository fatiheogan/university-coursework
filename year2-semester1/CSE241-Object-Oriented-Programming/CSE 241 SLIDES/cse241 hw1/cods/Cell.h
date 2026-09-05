#ifndef CELL_H
#define CELL_H

#include <string>

class Cell {
private:
    std::string value;    // Hücrenin hesaplanan değeri (string olarak saklanır)
    std::string formula;  // Hücrede yer alan formül (varsa)

    bool isFormula;       // Hücrenin bir formül içerip içermediğini belirtir

public:
    Cell() : value(""), formula(""), isFormula(false) {}

    // Getter ve Setter fonksiyonları
    const std::string& getValue() const { return value; }
    void setValue(const std::string& newValue) {
        value = newValue;
        /* Eğer yeni değer bir formül değilse (örneğin, '=' ile başlamıyorsa),
           hücre formül hücresi olarak işaretlenmez */
      
    }
    const std::string& getFormula() const { return formula; }
    void setFormula(const std::string& newFormula) {
        formula = newFormula;
        isFormula = true; // Yeni bir formül atanıyorsa, bu bir formül hücresidir
    }

    // Hücrenin bir formül içerip içermediğini döner
    bool isFormulaCell() const { return isFormula; }
};

#endif // CELL_H

