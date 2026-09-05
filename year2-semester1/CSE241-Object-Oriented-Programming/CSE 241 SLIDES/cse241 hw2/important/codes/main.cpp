#include "Spreadsheet.h"
#include "AnsiTerminal.h"
#include "FileManager.h"
#include <iostream>
#include <string>
#include <limits>

void displayMenu() {
    std::cout << "\033[2J\033[H"; // Terminali temizle
    std::cout << "=== Main Menu ===\n";
    std::cout << "1) Display Spreadsheet\n";
    std::cout << "2) Save to CSV\n";
    std::cout << "3) Load from CSV\n";
    std::cout << "4) Exit\n";
    std::cout << "Select an option: ";
}

void displaySpreadsheet(Spreadsheet& sheet, AnsiTerminal& terminal) {
    bool isSpreadsheetRunning = true;
    while (isSpreadsheetRunning) {
        // Tabloyu çiz
        sheet.printSpreadsheet(terminal);

        // Kullanıcı girdisini al
        char key = terminal.getSpecialKey();

        if (key == 'q') { // Menüye dön
            isSpreadsheetRunning = false;
            return; // Ana menüye dön
        }

        // Özel bir hareket girdisi mi?
        if (key == 'U' || key == 'D' || key == 'L' || key == 'R') {
            sheet.moveSelection(key); // İmleci hareket ettir
        } 
        // Girdi doğrudan içerik mi? (alfabetik ya da sayısal bir karakter)
        else if (std::isalnum(key) || std::ispunct(key)) {
            std::string content;
            content += key; // İlk karakteri ekle
            terminal.printAt(15, 1, ": ");

            // Kullanıcının geri dönmek için q basması durumunda süreci kes
            std::getline(std::cin, content);
            if (content == "q") {
                isSpreadsheetRunning = false;
                return;
            }

            sheet.setCellContent(sheet.getSelectedRow(), sheet.getSelectedCol(), content);
        }
    }
}

int main() {
    AnsiTerminal terminal;
    Spreadsheet sheet(10, 10); // 10x10 tablo oluştur
    bool isRunning = true;

    while (isRunning) {
        displayMenu();

        int choice;
        std::cout << "\033[32m"; // Yeşil renk
        if (!(std::cin >> choice)) { // Sayısal giriş kontrolü
            std::cin.clear(); // Hata durumunu temizle
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Geçersiz girdiyi atla
            std::cout << "\033[31mInvalid input. Please enter a number.\033[0m\n"; // Kırmızı renk hata mesajı
            continue;
        }
        std::cout << "\033[0m"; // Varsayılan renk
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Tamponu temizle

        switch (choice) {
            case 1:
                displaySpreadsheet(sheet, terminal); // Display Spreadsheet işlemi
                break;

            case 2: {
                std::string filename;
                std::cout << "Enter filename to save as CSV: ";
                std::cout << "\033[33m"; // Sarı renk
                std::cin >> filename;
                std::cout << "\033[0m"; // Varsayılan renk
                FileManager::saveToCSV(sheet, filename);
                std::cout << "Spreadsheet saved to " << filename << "\n";
                std::cout << "Press Enter to return to menu...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Tamponu temizle
                break;
            }

            case 3: {
                std::string filename;
                std::cout << "Enter filename to load from CSV: ";
                std::cout << "\033[33m"; // Sarı renk
                std::cin >> filename;
                std::cout << "\033[0m"; // Varsayılan renk
                FileManager::loadFromCSV(sheet, filename);
                std::cout << "Spreadsheet loaded from " << filename << "\n";
                std::cout << "Press Enter to return to menu...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Tamponu temizle
                break;
            }

            case 4:
                isRunning = false;
                std::cout << "Exiting program. Goodbye!\n";
                break;

            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
        }
    }

    return 0;
}

