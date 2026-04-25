#include "admin.h"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

using namespace std;

namespace {
struct LoginUser {
    UserRecord record;
    string role;
};

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int realChoice() {
    int choice;
    cout << "Nhap lua chon: ";

    if (!(cin >> choice)) {
        clearInput();
        return -1;
    }

    clearInput();
    return choice;
}

string getEmailLevel(const string& email) {
    const size_t atPos = email.find('@');
    if (atPos < 2) {
        return "";
    }

    const string level = email.substr(atPos - 2, 2);
    if (!isdigit(static_cast<unsigned char>(level[0])) ||
        !isdigit(static_cast<unsigned char>(level[1]))) {
        return "";
    }

    return level;
}

string roleFromEmail(const string& email) {
    const string level = getEmailLevel(email);

    if (level == "01") {
        return "admin";
    }
    if (level == "02") {
        return "teacher";
    }
    if (level == "03") {
        return "student";
    }

    return "";
}

string getRecordRole(const UserRecord& record) {
    if (record.fields.empty()) {
        return "";
    }

    return record.fields.back();
}

bool matchLogin(const UserRecord& record, const string& email, const string& password) {
    return record.fields.size() >= 4 &&
           record.fields[2] == email &&
           record.fields[3] == password;
}

bool findUserInList(const vector<UserRecord>& users,
                    const string& email,
                    const string& password,
                    LoginUser& loginUser) {
    for (const UserRecord& user : users) {
        if (matchLogin(user, email, password)) {
            loginUser.record = user;
            loginUser.role = roleFromEmail(email);
            if (loginUser.role.empty()) {
                loginUser.role = getRecordRole(user);
            }
            return true;
        }
    }

    return false;
}

bool login(const string& baseDir, LoginUser& loginUser) {
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

    return findUserInList(loadUserList(baseDir + "/admin.csv"), email, password, loginUser) ||
           findUserInList(loadUserList(baseDir + "/teacher.csv"), email, password, loginUser) ||
           findUserInList(loadUserList(baseDir + "/student.csv"), email, password, loginUser);
}

void openMenuByRole(const string& baseDir, const LoginUser& loginUser) {
    cout << "\nDang nhap thanh cong. Quyen truy cap: " << loginUser.role << "\n";

    if (loginUser.role == "admin") {
        runAdminMenu(baseDir);
    } else if (loginUser.role == "teacher") {
        runTeacherMenu(baseDir);
    } else if (loginUser.role == "student") {
        runStudentMenu(baseDir);
    } else {
        cout << "Khong xac dinh duoc quyen truy cap cua tai khoan nay.\n";
    }
}
}  

int main() {
    const string baseDir = ".";

    while (true) {
        LoginUser loginUser;

        if (login(baseDir, loginUser)) {
            openMenuByRole(baseDir, loginUser);
        } else {
            cout << "\nEmail hoac mat khau khong dung.\n";
        }

        cout << "\n1. Dang nhap lai\n";
        cout << "0. Thoat chuong trinh\n";
        cout << "9. Thoat chuong trinh\n";

        const int choice = realChoice();
        if (choice == 0 || choice == 9) {
            cout << "Da thoat chuong trinh.\n";
            return 0;
        }
    }
}
