#ifndef ADMIN_H
#define ADMIN_H

#include <string>
#include <vector>
using namespace std;

struct UserAdminRecord {
    std::vector<std::string> fields;
};

std::vector<UserAdminRecord> loadAdminUserList(const std::string& filePath);
void printUserList(const std::string& title, const std::vector<UserAdminRecord>& records);
void runUserListMenu(const std::string& baseDir = ".");
void runStudentMenu(const std::string& baseDir = ".");
void runTeacherMenu(const std::string& baseDir = ".");
void runAdminMenu(const std::string& baseDir = ".");

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
