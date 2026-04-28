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

std::string getField(const UserAdminRecord& record, size_t index) {
    if (index >= record.fields.size()) {
        return "";
    }

    return record.fields[index];
}

std::string findTeacherName(const std::vector<UserAdminRecord>& teachers, const std::string& teacherId) {
    for (const UserAdminRecord& teacher : teachers) {
        if (getField(teacher, 0) == teacherId) {
            return getField(teacher, 1);
        }
    }

    return "Khong tim thay giao vien";
}

int countEnrollmentsByCourse(const std::vector<UserAdminRecord>& enrollments, const std::string& courseId) {
    int count = 0;

    for (const UserAdminRecord& enrollment : enrollments) {
        if (getField(enrollment, 2) == courseId) {
            ++count;
        }
    }

    return count;
}

bool recordIdExists(const std::vector<UserAdminRecord>& records, const std::string& id) {
    for (const UserAdminRecord& record : records) {
        if (getField(record, 0) == id) {
            return true;
        }
    }

    return false;
}

std::string readRequiredText(const std::string& label) {
    std::string value;

    while (true) {
        std::cout << label;
        std::getline(std::cin, value);
        value = trim(value);

        if (!value.empty()) {
            return value;
        }

        std::cout << "Gia tri khong duoc de trong.\n";
    }
}

int readPositiveNumber(const std::string& label) {
    std::string value;

    while (true) {
        std::cout << label;
        std::getline(std::cin, value);
        value = trim(value);

        try {
            const int number = std::stoi(value);
            if (number > 0) {
                return number;
            }
        } catch (...) {
        }

        std::cout << "Vui long nhap so nguyen duong.\n";
    }
}

int toIntSafe(const std::string& value) {
    try {
        return std::stoi(trim(value));
    } catch (...) {
        return 0;
    }
}

bool appendCourse(const std::string& filePath, const std::vector<std::string>& fields) {
    std::ofstream fout(filePath, std::ios::app);
    if (!fout.is_open()) {
        std::cerr << "Khong ghi duoc file: " << filePath << '\n';
        return false;
    }

    for (size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) {
            fout << " | ";
        }
        fout << fields[i];
    }
    fout << '\n';
    return true;
}

void printCourseEnrollmentList(const std::string& baseDir) {
    const std::vector<UserAdminRecord> courses = loadAdminUserList(joinPath(baseDir, "course.csv"));
    const std::vector<UserAdminRecord> enrollments = loadAdminUserList(joinPath(baseDir, "enrollment.csv"));
    const std::vector<UserAdminRecord> teachers = loadAdminUserList(joinPath(baseDir, "teacher.csv"));

    std::cout << "\n===== DANH SACH KHOA HOC VA DANG KY =====\n";

    if (courses.empty()) {
        std::cout << "Chua co du lieu khoa hoc.\n";
        return;
    }

    for (const UserAdminRecord& course : courses) {
        const std::string courseId = getField(course, 0);
        const std::string courseName = getField(course, 1);
        const std::string credits = getField(course, 2);
        const std::string teacherId = getField(course, 3);
        const int maxStudents = toIntSafe(getField(course, 4));
        const std::string feePerCredit = getField(course, 5);
        const std::string startDate = getField(course, 6);
        const std::string endDate = getField(course, 7);
        const std::string teacherName = findTeacherName(teachers, teacherId);
        const int registeredCount = countEnrollmentsByCourse(enrollments, courseId);
        const bool isClosed = maxStudents > 0 && registeredCount >= maxStudents;

        std::cout << "\nMa khoa hoc: " << courseId << '\n';
        std::cout << "Ten khoa hoc: " << courseName << '\n';
        std::cout << "So tin chi: " << credits << '\n';
        std::cout << "Giao vien: " << teacherName << " (" << teacherId << ")\n";
        std::cout << "So sinh vien: " << registeredCount << "/" << maxStudents << '\n';
        std::cout << "Tien hoc moi tin chi: " << feePerCredit << '\n';
        std::cout << "Ngay bat dau: " << startDate << '\n';
        std::cout << "Ngay ket thuc: " << endDate << '\n';
        std::cout << "Trang thai: " << (isClosed ? "Da dong" : "Dang mo") << '\n';
    }
}

void createCourse(const std::string& baseDir) {
    const std::string coursePath = joinPath(baseDir, "course.csv");
    const std::string teacherPath = joinPath(baseDir, "teacher.csv");
    const std::vector<UserAdminRecord> courses = loadAdminUserList(coursePath);
    const std::vector<UserAdminRecord> teachers = loadAdminUserList(teacherPath);

    std::cout << "\n===== TAO KHOA HOC MOI =====\n";

    std::string courseId;
    while (true) {
        courseId = readRequiredText("Ma khoa hoc: ");
        if (!recordIdExists(courses, courseId)) {
            break;
        }
        std::cout << "Ma khoa hoc da ton tai. Vui long nhap ma khac.\n";
    }

    const std::string courseName = readRequiredText("Ten khoa hoc: ");
    const int credits = readPositiveNumber("So tin chi: ");

    std::string teacherId;
    while (true) {
        teacherId = readRequiredText("Ma giao vien phu trach: ");
        if (recordIdExists(teachers, teacherId)) {
            break;
        }
        std::cout << "Khong tim thay giao vien voi ma nay.\n";
    }

    const int maxStudents = readPositiveNumber("So luong sinh vien toi da: ");
    const int feePerCredit = readPositiveNumber("Tien hoc moi tin chi: ");
    const std::string startDate = readRequiredText("Ngay bat dau (dd/mm/yyyy): ");
    const std::string endDate = readRequiredText("Ngay ket thuc (dd/mm/yyyy): ");

    if (appendCourse(coursePath, {
            courseId,
            courseName,
            std::to_string(credits),
            teacherId,
            std::to_string(maxStudents),
            std::to_string(feePerCredit),
            startDate,
            endDate
        })) {
        std::cout << "Da tao khoa hoc thanh cong.\n";
    }
}

void runCourseMenu(const std::string& baseDir) {
    while (true) {
        std::cout << "\n===== QUAN LY KHOA HOC =====\n";
        std::cout << "1. Xem khoa hoc va thong tin dang ky\n";
        std::cout << "2. Tao khoa hoc moi\n";
        std::cout << "0. Quay lai menu admin\n";
        std::cout << "9. Thoat chuong trinh\n";

        switch (readChoice()) {
            case 1:
                printCourseEnrollmentList(baseDir);
                pauseScreen();
                break;
            case 2:
                createCourse(baseDir);
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

std::vector<UserAdminRecord> loadAdminUserList(const std::string& filePath) {
    std::ifstream fin(filePath);
    std::vector<UserAdminRecord> records;
    std::string line;

    if (!fin.is_open()) {
        std::cerr << "Khong mo duoc file: " << filePath << '\n';
        return records;
    }

    while (std::getline(fin, line)) {
        if (trim(line).empty()) {
            continue;
        }

        UserAdminRecord record;
        record.fields = splitByPipe(line);
        records.push_back(record);
    }

    return records;
}

void printUserList(const std::string& title, const std::vector<UserAdminRecord>& records) {
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
    const std::vector<UserAdminRecord> students = loadAdminUserList(joinPath(baseDir, "student.csv"));
    const std::vector<UserAdminRecord> teachers = loadAdminUserList(joinPath(baseDir, "teacher.csv"));
    const std::vector<UserAdminRecord> admins = loadAdminUserList(joinPath(baseDir, "admin.csv"));

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
                printUserList("Danh sach sinh vien", loadAdminUserList(joinPath(baseDir, "student.csv")));
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
                printUserList("Danh sach giao vien", loadAdminUserList(joinPath(baseDir, "teacher.csv")));
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
