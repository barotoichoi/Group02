#include "admin.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <cstdlib>

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

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int readChoice() {
    int choice;
    std::cout << "Nhap lua chon: ";

    if (!(std::cin >> choice)) {
        clearInput();
        return -1;
    }

    clearInput();
    return choice;
}

void pauseScreen() {
    std::cout << "\nNhan Enter de tiep tuc...";
    std::cin.get();
}

void exitProgram() {
    std::cout << "Da thoat chuong trinh.\n";
    std::exit(0);
}

std::string getField(const UserRecord& record, size_t index) {
    if (index >= record.fields.size()) {
        return "";
    }

    return record.fields[index];
}

std::string findTeacherName(const std::vector<UserRecord>& teachers, const std::string& teacherId) {
    for (const UserRecord& teacher : teachers) {
        if (getField(teacher, 0) == teacherId) {
            return getField(teacher, 1);
        }
    }

    return "Khong tim thay giao vien";
}

int countEnrollmentsByCourse(const std::vector<UserRecord>& enrollments, const std::string& courseId) {
    int count = 0;

    for (const UserRecord& enrollment : enrollments) {
        if (getField(enrollment, 2) == courseId) {
            ++count;
        }
    }

    return count;
}

std::string formatDate(const std::string& date) {
    std::string result;

    for (char ch : date) {
        if (ch == '/' || ch == '-') {
            result += "-";
        } else {
            result += ch;
        }
    }

    return result;
}

void printCourseEnrollmentList(const std::string& baseDir) {
    const std::vector<UserRecord> courses = loadUserList(joinPath(baseDir, "course.csv"));
    const std::vector<UserRecord> enrollments = loadUserList(joinPath(baseDir, "enrollment.csv"));
    const std::vector<UserRecord> teachers = loadUserList(joinPath(baseDir, "teacher.csv"));

    std::cout << "\n===== DANH SACH KHOA HOC VA DANG KY =====\n";

    if (courses.empty()) {
        std::cout << "Chua co du lieu khoa hoc.\n";
        return;
    }

    for (const UserRecord& course : courses) {
        const std::string courseId = getField(course, 0);
        const std::string courseName = getField(course, 1);
        const std::string teacherId = getField(course, 3);
        const std::string teacherName = findTeacherName(teachers, teacherId);
        const int registeredCount = countEnrollmentsByCourse(enrollments, courseId);

        std::cout << "\nMa khoa hoc: " << courseId << '\n';
        std::cout << "Ten khoa hoc: " << courseName << '\n';
        std::cout << "Giao vien: " << teacherName << " (" << teacherId << ")\n";
        std::cout << "So sinh vien dang ky: " << registeredCount << '\n';

        bool hasEnrollment = false;
        for (const UserRecord& enrollment : enrollments) {
            if (getField(enrollment, 2) == courseId) {
                hasEnrollment = true;
                std::cout << "Ngay dang ky: " << formatDate(getField(enrollment, 3)) << '\n';
                std::cout << "Trang thai: " << getField(enrollment, 4) << '\n';
            }
        }

        if (!hasEnrollment) {
            std::cout << "Chua co sinh vien dang ky.\n";
        }
    }
}

void runCourseMenu(const std::string& baseDir) {
    while (true) {
        std::cout << "\n===== QUAN LY KHOA HOC =====\n";
        std::cout << "1. Xem khoa hoc va thong tin dang ky\n";
        std::cout << "0. Quay lai menu admin\n";
        std::cout << "9. Thoat chuong trinh\n";

        switch (readChoice()) {
            case 1:
                printCourseEnrollmentList(baseDir);
                pauseScreen();
                break;
            case 0:
                return;
            case 9:
                exitProgram();
            default:
                std::cout << "Lua chon khong hop le. Vui long chon lai.\n";
                break;
        }
    }
}

void runPaymentMenu() {
    while (true) {
        std::cout << "\n===== QUAN LY KHOAN THANH TOAN =====\n";
        std::cout << "1. Xem danh sach khoan thanh toan\n";
        std::cout << "0. Quay lai menu admin\n";
        std::cout << "9. Thoat chuong trinh\n";

        switch (readChoice()) {
            case 1:
                std::cout << "Chuc nang xem danh sach khoan thanh toan dang duoc phat trien.\n";
                pauseScreen();
                break;
            case 0:
                return;
            case 9:
                exitProgram();
            default:
                std::cout << "Lua chon khong hop le. Vui long chon lai.\n";
                break;
        }
    }
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
        std::cout << "9. Thoat chuong trinh\n";
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
            case 9:
                exitProgram();
            default:
                std::cout << "Lua chon khong hop le. Vui long chon lai.\n";
                break;
        }
    }
}

void runStudentMenu(const std::string& baseDir) {
    while (true) {
        std::cout << "\n===== QUAN LY SINH VIEN =====\n";
        std::cout << "1. Xem danh sach sinh vien\n";
        std::cout << "0. Quay lai\n";
        std::cout << "9. Thoat chuong trinh\n";

        switch (readChoice()) {
            case 1:
                printUserList("Danh sach sinh vien", loadUserList(joinPath(baseDir, "student.csv")));
                pauseScreen();
                break;
            case 0:
                return;
            case 9:
                exitProgram();
            default:
                std::cout << "Lua chon khong hop le. Vui long chon lai.\n";
                break;
        }
    }
}

void runTeacherMenu(const std::string& baseDir) {
    while (true) {
        std::cout << "\n===== QUAN LY GIAO VIEN =====\n";
        std::cout << "1. Xem danh sach giao vien\n";
        std::cout << "0. Quay lai\n";
        std::cout << "9. Thoat chuong trinh\n";

        switch (readChoice()) {
            case 1:
                printUserList("Danh sach giao vien", loadUserList(joinPath(baseDir, "teacher.csv")));
                pauseScreen();
                break;
            case 0:
                return;
            case 9:
                exitProgram();
            default:
                std::cout << "Lua chon khong hop le. Vui long chon lai.\n";
                break;
        }
    }
}

void runAdminMenu(const std::string& baseDir) {
    while (true) {
        std::cout << "\n========== MENU ADMIN ==========\n";
        std::cout << "1. Quan ly sinh vien\n";
        std::cout << "2. Quan ly giao vien\n";
        std::cout << "3. Quan ly khoa hoc\n";
        std::cout << "4. Quan ly khoan thanh toan\n";
        std::cout << "0. Dang xuat\n";
        std::cout << "9. Thoat chuong trinh\n";

        switch (readChoice()) {
            case 1:
                runStudentMenu(baseDir);
                break;
            case 2:
                runTeacherMenu(baseDir);
                break;
            case 3:
                runCourseMenu(baseDir);
                break;
            case 4:
                runPaymentMenu();
                break;
            case 0:
                return;
            case 9:
                exitProgram();
            default:
                std::cout << "Lua chon khong hop le. Vui long chon lai.\n";
                break;
        }
    }
}
