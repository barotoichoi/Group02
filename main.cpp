#include "admin.h"
#include "student.h"
#include "teacher.h"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

using namespace std;

namespace {
struct LoginUser {
    string id;
    string name;
    string role;
};

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int readChoice() {
    int choice;
    cout << "Nhap lua chon: ";

    if (!(cin >> choice)) {
        clearInput();
        return -1;
    }

    clearInput();
    return choice;
}

bool matchFields(const vector<string>& fields, const string& email, const string& password) {
    return fields.size() >= 4 && fields[2] == email && fields[3] == password;
}

bool findStudent(const string& baseDir,
                 const string& email,
                 const string& password,
                 LoginUser& loginUser) {
    const vector<UserStudentRecord> students = loadUserList(baseDir + "/student.csv");

    for (const UserStudentRecord& student : students) {
        if (matchFields(student.fields, email, password)) {
            loginUser.id = student.fields[0];
            loginUser.name = student.fields[1];
            loginUser.role = "student";
            return true;
        }
    }

    return false;
}

bool findAdmin(const string& baseDir,
               const string& email,
               const string& password,
               LoginUser& loginUser) {
    const vector<UserAdminRecord> admins = loadAdminUserList(joinPath(baseDir, "admin.csv"));

    for (const UserAdminRecord& admin : admins) {
        if (matchFields(admin.fields, email, password)) {
            loginUser.id = admin.fields[0];
            loginUser.name = admin.fields[1];
            loginUser.role = "admin";
            return true;
        }
    }

    return false;
}

bool findTeacher(const string& baseDir,
                 const string& email,
                 const string& password,
                 LoginUser& loginUser) {
    const vector<UserTeacherRecord> teachers = loadCsv(joinPath(baseDir, "teacher.csv"));

    for (const UserTeacherRecord& teacher : teachers) {
        if (matchFields(teacher.fields, email, password)) {
            loginUser.id = teacher.fields[0];
            loginUser.name = teacher.fields[1];
            loginUser.role = "teacher";
            return true;
        }
    }

    return false;
}

bool loginSystem(const string& baseDir, LoginUser& loginUser) {
    string email;
    string password;

    cout << "\n========== DANG NHAP HE THONG ==========\n";
    cout << "Email (nhap 0 de thoat): ";
    getline(cin, email);

    if (email == "0") {
        cout << "Da thoat chuong trinh.\n";
        exit(0);
    }

    cout << "Mat khau: ";
    getline(cin, password);

    return findAdmin(baseDir, email, password, loginUser) ||
           findTeacher(baseDir, email, password, loginUser) ||
           findStudent(baseDir, email, password, loginUser);
}

void openMenuByRole(const string& baseDir, const LoginUser& loginUser) {
    cout << "\nDang nhap thanh cong: " << loginUser.name << "\n";
    cout << "Quyen truy cap: " << loginUser.role << "\n";

    if (loginUser.role == "admin") {
        runAdminMenu(baseDir);
    } else if (loginUser.role == "teacher") {
        runTeacherMenu(baseDir, loginUser.id);
    } else if (loginUser.role == "student") {
        // Use course-based student menu (matches requested options)
        studentCourseMenu(loginUser.id);
    }
}
}

int main() {
    const string baseDir = ".";

    while (true) {
        LoginUser loginUser;

        if (loginSystem(baseDir, loginUser)) {
            openMenuByRole(baseDir, loginUser);
        } else {
            cout << "\nEmail hoac mat khau khong dung.\n";
        }

        cout << "\n1. Dang nhap lai\n";
        cout << "0. Thoat chuong trinh\n";

        const int choice = readChoice();
        if (choice == 0) {
            cout << "Da thoat chuong trinh.\n";
            return 0;
        }
    }
}
