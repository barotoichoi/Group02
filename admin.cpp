#include "admin.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

namespace {
std::string trim(const std::string& value) {
    const size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }

    const size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::vector<std::string> splitByPipe(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string token;

    while (std::getline(ss, token, '|')) {
        fields.push_back(trim(token));
    }

    return fields;
}

std::string joinPath(const std::string& dir, const std::string& fileName) {
    if (dir.empty() || dir == ".") {
        return fileName;
    }

    const char last = dir.back();
    if (last == '/' || last == '\\') {
        return dir + fileName;
    }

    return dir + "/" + fileName;
}
}  // namespace

std::vector<UserRecord> loadUserList(const std::string& filePath) {
    std::ifstream fin(filePath);
    std::vector<UserRecord> records;
    std::string line;

    if (!fin.is_open()) {
        std::cerr << "Khong mo duoc file: " << filePath << '\n';
        return records;
    }

    while (std::getline(fin, line)) {
        if (trim(line).empty()) {
            continue;
        }

        UserRecord record;
        record.fields = splitByPipe(line);
        records.push_back(record);
    }

    return records;
}

void printUserList(const std::string& title, const std::vector<UserRecord>& records) {
    std::cout << "\n=== " << title << " (" << records.size() << " ban ghi) ===\n";

    for (size_t i = 0; i < records.size(); ++i) {
        std::cout << std::setw(2) << (i + 1) << ". ";
        for (size_t j = 0; j < records[i].fields.size(); ++j) {
            if (j > 0) {
                std::cout << " | ";
            }
            std::cout << records[i].fields[j];
        }
        std::cout << '\n';
    }
}

void runDanhSachNguoiDung(const std::string& baseDir) {
    const std::vector<UserRecord> students = loadUserList(joinPath(baseDir, "student.csv"));
    const std::vector<UserRecord> teachers = loadUserList(joinPath(baseDir, "teacher.csv"));
    const std::vector<UserRecord> admins = loadUserList(joinPath(baseDir, "admin.csv"));

    int choice = -1;
    while (true) {
        std::cout << "\n===== MENU DANH SACH =====\n";
        std::cout << "1. In danh sach Student\n";
        std::cout << "2. In danh sach Teacher\n";
        std::cout << "3. In danh sach Admin\n";
        std::cout << "4. In tat ca danh sach\n";
        std::cout << "0. Thoat\n";
        std::cout << "Nhap lua chon: ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Lua chon khong hop le. Vui long nhap lai.\n";
            continue;
        }

        switch (choice) {
            case 1:
                printUserList("Danh sach Student", students);
                break;
            case 2:
                printUserList("Danh sach Teacher", teachers);
                break;
            case 3:
                printUserList("Danh sach Admin", admins);
                break;
            case 4:
                printUserList("Danh sach Student", students);
                printUserList("Danh sach Teacher", teachers);
                printUserList("Danh sach Admin", admins);
                break;
            case 0:
                std::cout << "Da thoat menu danh sach.\n";
                return;
            default:
                std::cout << "Lua chon khong hop le. Vui long chon lai.\n";
                break;
        }
    }
}
