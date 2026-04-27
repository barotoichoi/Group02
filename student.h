#ifndef STUDENT_H
#define STUDENT_H

#include <string>
#include <vector>
using namespace std;

struct UserRecord {
    std::vector<std::string> fields;
};

std::vector<UserRecord> loadUserList(const std::string& filePath);
void printUserList(const std::string& title, const std::vector<UserRecord>& records);
void runDanhSachNguoiDung(const std::string& baseDir = ".");
void runStudentMenu(const std::string& baseDir = ".");
void runTeacherMenu(const std::string& baseDir = ".");
void runAdminMenu(const std::string& baseDir = ".");
void runLogin(const std::string& filePath);

class Admin {
public:
    int adminId;
    string name;
    string email;
    string password;
    string phone;
    string address;
    string role;
};

#endif
