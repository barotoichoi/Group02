#include "student.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

using namespace std;

namespace {
vector<string> split(const string& line) {
    vector<string> result;
    string token;
    stringstream ss(line);

    while (getline(ss, token, '|')) {
        result.push_back(token);
    }
    return result;
}


string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

string joinPath(const string& dir, const string& fileName) {
    if (dir.empty() || dir == ".") return fileName;
    char last = dir.back();
    if (last == '/' || last == '\\') return dir + fileName;
    return dir + "/" + fileName;
}

string getField(const vector<string>& fields, size_t index) {
    if (index >= fields.size()) return "";
    return fields[index];
}

int readChoice() {
    int choice;
    cout << "Nhap lua chon: ";

    if (!(cin >> choice)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return -1;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return choice;
}

void printStudentProfile(const string& baseDir, const string& studentId) {
    vector<UserStudentRecord> students = loadUserList(joinPath(baseDir, "student.csv"));

    for (const UserStudentRecord& student : students) {
        if (getField(student.fields, 0) == studentId) {
            cout << "\n===== THONG TIN SINH VIEN =====\n";
            cout << "ID: " << getField(student.fields, 0) << '\n';
            cout << "Ten: " << getField(student.fields, 1) << '\n';
            cout << "Email: " << getField(student.fields, 2) << '\n';
            cout << "So dien thoai: " << getField(student.fields, 4) << '\n';
            cout << "Dia chi: " << getField(student.fields, 5) << '\n';
            cout << "Nganh: " << getField(student.fields, 6) << '\n';
            cout << "Lop: " << getField(student.fields, 7) << '\n';
            return;
        }
    }

    cout << "Khong tim thay thong tin sinh vien.\n";
}

void printCourseList(const string& baseDir) {
    ifstream file(joinPath(baseDir, "course.csv"));
    if (!file.is_open()) {
        cout << "Khong mo duoc file course.csv!\n";
        return;
    }

    cout << "\n===== DANH SACH KHOA HOC =====\n";
    string line;
    while (getline(file, line)) {
        if (trim(line).empty()) continue;

        vector<string> fields = split(line);
        for (string& field : fields) {
            field = trim(field);
        }

        cout << "Ma: " << getField(fields, 0)
             << " | Ten: " << getField(fields, 1)
             << " | Tin chi: " << getField(fields, 2)
             << " | Hoc phi: " << getField(fields, 5)
             << '\n';
    }
}

void printEnrollmentList(const string& baseDir, const string& studentId) {
    ifstream file(joinPath(baseDir, "enrollment.csv"));
    if (!file.is_open()) {
        cout << "Khong mo duoc file enrollment.csv!\n";
        return;
    }

    cout << "\n===== KHOA HOC DA DANG KY =====\n";
    bool found = false;
    string line;
    while (getline(file, line)) {
        if (trim(line).empty()) continue;

        vector<string> fields = split(line);
        for (string& field : fields) {
            field = trim(field);
        }

        if (getField(fields, 1) == studentId) {
            cout << "Ma dang ky: " << getField(fields, 0)
                 << " | Ma khoa hoc: " << getField(fields, 2)
                 << " | Ngay dang ky: " << getField(fields, 3)
                 << " | Hoc phi: " << getField(fields, 4)
                 << " | Diem: " << getField(fields, 5)
                 << '\n';
            found = true;
        }
    }

    if (!found) {
        cout << "Sinh vien chua dang ky khoa hoc nao.\n";
    }
}
}


vector<UserStudentRecord> loadUserList(const string& filePath) {
    vector<UserStudentRecord> users;
    ifstream file(filePath);
    string line;

    if (!file.is_open()) {
        cout << "Khong mo duoc file!\n";
        return users;
    }

    while (getline(file, line)) {
        if (line.empty()) continue;

        vector<string> fields = split(line);
        for (auto& field : fields) {
            field = trim(field);
        }

        if (fields.size() < 9) continue;

        UserStudentRecord user;
        user.fields = std::move(fields);
        users.push_back(user);
    }

    return users;
}

void runStudentMenu(const string& baseDir, const string& studentId) {
    while (true) {
        cout << "\n========== MENU SINH VIEN ==========\n";
        cout << "1. Xem thong tin ca nhan\n";
        cout << "2. Xem danh sach khoa hoc\n";
        cout << "3. Xem khoa hoc da dang ky\n";
        cout << "0. Dang xuat\n";

        switch (readChoice()) {
            case 1:
                printStudentProfile(baseDir, studentId);
                break;
            case 2:
                printCourseList(baseDir);
                break;
            case 3:
                printEnrollmentList(baseDir, studentId);
                break;
            case 0:
                cout << "Dang xuat...\n";
                return;
            default:
                cout << "Lua chon khong hop le.\n";
                break;
        }
    }
}

