#include "student.h"
#include <iostream>
#include <string>

int main() {
    const std::string studentFile = "student.csv";
    std::cout << "Chay mainstudent - su dung file: " << studentFile << std::endl;

    auto users = loadUserList(studentFile);
    if (users.empty()) {
        std::cout << "Khong tim thay ban ghi sinh vien hoac khong mo duoc file.\n";
        return 1;
    }

    std::string email, password;
    std::cout << "===== DANG NHAP SINH VIEN =====\n";
    std::cout << "Email: ";
    std::getline(std::cin, email);
    std::cout << "Mat khau: ";
    std::getline(std::cin, password);

    for (const auto& u : users) {
        if (u.fields.size() >= 9 &&
            u.fields[2] == email &&
            u.fields[3] == password &&
            u.fields[8] == "student") {

            std::cout << "\nDang nhap thanh cong!\n";
            std::cout << "----- THONG TIN CA NHAN -----\n";
            std::cout << "ID: " << u.fields[0] << "\n";
            std::cout << "Ten: " << u.fields[1] << "\n";
            std::cout << "Email: " << u.fields[2] << "\n";
            std::cout << "SDT: " << u.fields[4] << "\n";
            std::cout << "Dia chi: " << u.fields[5] << "\n";
            std::cout << "Nganh: " << u.fields[6] << "\n";
            std::cout << "Lop: " << u.fields[7] << "\n\n";

            runStudentMenu("."); // gọi menu sinh viên (nếu đã được hiện thực)
            return 0;
        }
    }

    std::cout << "\nSai email hoac mat khau!\n";
    return 0;
}