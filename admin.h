#ifndef ADMIN_H
#define ADMIN_H

#include <string>
#include <vector>
using namespace std;

struct UserRecord {
    std::vector<std::string> fields;
};

std::vector<UserRecord> loadUserList(const std::string& filePath);
void printUserList(const std::string& title, const std::vector<UserRecord>& records);
void runDanhSachNguoiDung(const std::string& baseDir = ".");

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
