#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Course {
    string id;
    string name;
    int credits = 0;
    string phone;
    int feePerCredit = 0;
    int totalFee = 0;
    string startDate;
    string endDate;
};

string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) {
        return "";
    }
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

vector<string> splitPipe(const string& line) {
    vector<string> parts;
    string part;
    stringstream ss(line);

    while (getline(ss, part, '|')) {
        parts.push_back(trim(part));
    }
    return parts;
}

int toIntSafe(const string& s) {
    try {
        return stoi(s);
    } catch (...) {
        return 0;
    }
}

int main() {
    const string filePath = "course.csv";
    ifstream fin(filePath);
    if (!fin.is_open()) {
        cerr << "Khong mo duoc file: " << filePath << '\n';
        return 1;
    }

    vector<Course> courses;
    string line;
    while (getline(fin, line)) {
        if (trim(line).empty()) {
            continue;
        }

        vector<string> fields = splitPipe(line);
        if (fields.size() < 7) {
            continue;
        }

        Course c;
        c.id = fields[0];
        c.name = fields[1];
        c.credits = toIntSafe(fields[2]);
        c.phone = fields[3];
        c.feePerCredit = toIntSafe(fields[4]);

        // Ho tro 2 dinh dang:
        // 7 cot: ... | HocPhi/TC | BatDau | KetThuc
        // 8 cot: ... | HocPhi/TC | TongHocPhi | BatDau | KetThuc
        if (fields.size() >= 8) {
            c.totalFee = toIntSafe(fields[5]);
            c.startDate = fields[6];
            c.endDate = fields[7];
        } else {
            c.startDate = fields[5];
            c.endDate = fields[6];
        }

        if (c.totalFee <= 0) {
            c.totalFee = c.credits * c.feePerCredit;
        }
        courses.push_back(c);
    }

    if (courses.empty()) {
        cout << "Khong co du lieu hoc phi trong file.\n";
        return 0;
    }

    cout << left
         << setw(8) << "MaMon"
         << setw(22) << "TenMon"
         << setw(10) << "TinChi"
         << setw(14) << "HocPhi/TC"
         << setw(12) << "TongPhi"
         << setw(14) << "BatDau"
         << setw(14) << "KetThuc"
         << '\n';

    cout << string(94, '-') << '\n';

    long long grandTotal = 0;
    for (const Course& c : courses) {
        long long total = c.totalFee;
        grandTotal += total;

        cout << left
             << setw(8) << c.id
             << setw(22) << c.name
             << setw(10) << c.credits
             << setw(14) << c.feePerCredit
             << setw(12) << total
             << setw(14) << c.startDate
             << setw(14) << c.endDate
             << '\n';
    }

    cout << string(94, '-') << '\n';
    cout << "Tong hoc phi tat ca mon: " << grandTotal << '\n';

    return 0;
}
