#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>
#include "Spreadsheet.h"

class FileManager {
public:
    static void saveToCSV(const Spreadsheet& sheet, const std::string& filename);
    static void loadFromCSV(Spreadsheet& sheet, const std::string& filename);
};

#endif // FILEMANAGER_H

