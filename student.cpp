#include "student.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;


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
    size_t first = str.find_first_not_of(' ');
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, last - first + 1);
}


vector<Student> loadStudents(const string& filePath) {
    vector<Student> students;
    ifstream file(filePath);
    string line;

    if (!file.is_open()) {
        cout << "Khong mo duoc file!\n";
        return students;
    }

    while (getline(file, line)) {
        if (line.empty()) continue;

        Student s;
        s.fields = split(line);
        for (auto& field : s.fields) {
            field = trim(field);
        }
        students.push_back(s);
    }

    return students;
}


void printProfile(const Student& s) {
    cout << "\n===== THONG TIN CA NHAN =====\n";
    cout << "ID: " << s.fields[0] << endl;
    cout << "Ten: " << s.fields[1] << endl;
    cout << "Email: " << s.fields[2] << endl;
    cout << "So dien thoai: " << s.fields[4] << endl;
    cout << "Dia chi: " << s.fields[5] << endl;
    cout << "Nganh: " << s.fields[6] << endl;
    cout << "Lop: " << s.fields[7] << endl;
}


void runLogin(const string& filePath) {
    vector<Student> students = loadStudents(filePath);

    string email, password;

    cout << "===== DANG NHAP =====\n";
    cout << "Email: ";
    getline(cin, email);

    cout << "Mat khau: ";
    getline(cin, password);

    for (const Student& s : students) {
        if (s.fields.size() >= 9 &&
            s.fields[2] == email &&
            s.fields[3] == password &&
            s.fields[8] == "student") {

            cout << "\nDang nhap thanh cong!\n";
            printProfile(s);
            return;
        }
    }

    cout << "\nSai email hoac mat khau!\n";
}
