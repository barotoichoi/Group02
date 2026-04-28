#include "teacher.h"
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

using namespace std;

string trimString(const string& value) {
    const size_t first = value.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    const size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

string joinPath(const string& dir, const string& fileName) {
    if (dir.empty() || dir == ".") return fileName;
    char last = dir.back();
    if (last == '/' || last == '\\') return dir + fileName;
    return dir + "/" + fileName;
}

vector<UserTeacherRecord> loadCsv(const string& filePath) {
    vector<UserTeacherRecord> records;
    ifstream fin(filePath);
    if (!fin.is_open()) return records;

    string line;
    bool firstLine = true;
    while (getline(fin, line)) {
        if (firstLine) {
            if (line.size() >= 3 &&
                static_cast<unsigned char>(line[0]) == 0xEF &&
                static_cast<unsigned char>(line[1]) == 0xBB &&
                static_cast<unsigned char>(line[2]) == 0xBF) {
                line = line.substr(3);
            }
            firstLine = false;
        }
        if (trimString(line).empty()) continue;
        UserTeacherRecord record;
        stringstream ss(line);
        string token;
        while (getline(ss, token, '|')) {
            record.fields.push_back(trimString(token));
        }
        records.push_back(record);
    }
    return records;
}

string getField(const vector<string>& fields, size_t index) {
    if (index >= fields.size()) return "";
    return fields[index];
}

void saveCsv(const string& filePath, const vector<UserTeacherRecord>& records) {
    ofstream fout(filePath);
    if (!fout.is_open()) return;
    for (const auto& record : records) {
        for (size_t i = 0; i < record.fields.size(); ++i) {
            fout << record.fields[i];
            if (i < record.fields.size() - 1) fout << " | ";
        }
        fout << '\n';
    }
}

void viewTeachingSchedule(const string& baseDir, const string& teacherId) {
    vector<UserTeacherRecord> courses = loadCsv(joinPath(baseDir, "course.csv"));
    cout << "\n--- LICH DAY & CHI TIET KHOA HOC ---\n";

    bool hasCourse = false;
    for (const auto& c : courses) {
        if (c.fields.size() > 7 && c.fields[3] == teacherId) {
            if (trimString(c.fields[4]) == "pending") continue;

            cout << "- Ma: " << c.fields[0] << " | Ten: " << c.fields[1]
                 << "\n  So TC: " << c.fields[2] << " | Hoc phi: " << c.fields[5]
                 << "\n  Thoi gian: " << c.fields[6] << " den " << c.fields[7] << "\n";
            hasCourse = true;
        }
    }
    if (!hasCourse) cout << "Khong co lich day hop le.\n";
}

void inputGrades(const string& baseDir, const string& teacherId) {
    string cid, sid, grade;
    cout << "Ma khoa hoc: ";
    getline(cin, cid);
    cid = trimString(cid);

    vector<UserTeacherRecord> courses = loadCsv(joinPath(baseDir, "course.csv"));
    bool canInput = false;
    for (const auto& c : courses) {
        if (c.fields.size() > 4 && c.fields[0] == cid && c.fields[3] == teacherId) {
            if (trimString(c.fields[4]) == "pending") {
                cout << "=> Loi: Khoa hoc dang 'pending', khong the nhap diem!\n";
                return;
            }
            canInput = true;
            break;
        }
    }

    if (!canInput) {
        cout << "=> Loi: Ban khong phu trach khoa hoc nay!\n";
        return;
    }

    cout << "Ma SV: ";
    getline(cin, sid);
    sid = trimString(sid);
    cout << "Diem: ";
    getline(cin, grade);

    string ePath = joinPath(baseDir, "enrollment.csv");
    vector<UserTeacherRecord> enrolls = loadCsv(ePath);
    bool found = false;
    for (auto& e : enrolls) {
        if (e.fields.size() > 2 && e.fields[1] == sid && e.fields[2] == cid) {
            while (e.fields.size() <= 5) e.fields.push_back("");
            e.fields[5] = trimString(grade);
            found = true;
        }
    }

    if (found) {
        saveCsv(ePath, enrolls);
        cout << "=> Luu diem thanh cong!\n";
    } else {
        cout << "=> Loi: Khong tim thay ban ghi dang ky!\n";
    }
}

void updatePersonalInfo(const string& baseDir, const string& teacherId) {
    string path = joinPath(baseDir, "teacher.csv");
    vector<UserTeacherRecord> teachers = loadCsv(path);
    for (auto& t : teachers) {
        if (!t.fields.empty() && t.fields[0] == teacherId) {
            if (t.fields.size() == 8 && t.fields.back() == "teacher") {
                t.fields.insert(t.fields.begin() + 4, "");
            }
            string in;
            cout << "\n--- CAP NHAT THONG TIN ---\n";
            cout << "Ho ten moi: "; getline(cin, in);
            if (!trimString(in).empty()) t.fields[1] = trimString(in);
            cout << "Email moi: "; getline(cin, in);
            if (!trimString(in).empty()) t.fields[2] = trimString(in);
            cout << "Ngay sinh moi: "; getline(cin, in);
            if (!trimString(in).empty()) t.fields[4] = trimString(in);
            cout << "SDT moi: "; getline(cin, in);
            if (!trimString(in).empty()) t.fields[5] = trimString(in);
            saveCsv(path, teachers);
            cout << "=> Thanh cong!\n";
            return;
        }
    }
}

void viewStudentList(const string& baseDir, const string& teacherId) {
    string cid;
    cout << "Nhap ma khoa hoc: ";
    getline(cin, cid);
    cid = trimString(cid);

    vector<UserTeacherRecord> courses = loadCsv(joinPath(baseDir, "course.csv"));
    bool isMyCourse = false;
    for (const auto& c : courses) {
        if (c.fields.size() > 3 && c.fields[0] == cid && c.fields[3] == teacherId) {
            isMyCourse = true; break;
        }
    }
    if (!isMyCourse) {
        cout << "=> Loi: Ban khong phu trach khoa hoc nay!\n";
        return;
    }

    vector<UserTeacherRecord> enrolls = loadCsv(joinPath(baseDir, "enrollment.csv"));
    vector<UserTeacherRecord> students = loadCsv(joinPath(baseDir, "student.csv"));
    cout << "\n--- DANH SACH SINH VIEN ---\n";
    bool hasStudent = false;
    for (const auto& e : enrolls) {
        if (e.fields.size() > 2 && e.fields[2] == cid) {
            for (const auto& s : students) {
                if (s.fields.size() > 1 && s.fields[0] == e.fields[1]) {
                    cout << s.fields[0] << " | " << s.fields[1] << "\n";
                    hasStudent = true; break;
                }
            }
        }
    }
    if (!hasStudent) cout << "Chua co sinh vien dang ky.\n";
}

void changePassword(const string& baseDir, const string& teacherId) {
    string path = joinPath(baseDir, "teacher.csv");
    vector<UserTeacherRecord> teachers = loadCsv(path);
    for (auto& t : teachers) {
        if (!t.fields.empty() && t.fields[0] == teacherId) {
            string oldP, newP;
            cout << "Mat khau hien tai: "; getline(cin, oldP);
            if (t.fields[3] != trimString(oldP)) {
                cout << "=> Sai mat khau!\n"; return;
            }
            cout << "Mat khau moi: "; getline(cin, newP);
            if (!trimString(newP).empty()) {
                t.fields[3] = trimString(newP);
                saveCsv(path, teachers);
                cout << "=> Doi mat khau thanh cong!\n";
            }
            return;
        }
    }
}

struct TeacherNotification {
    size_t rowIndex;
    string courseId;
    string courseName;
    string content;
};

void viewTeacherNotifications(const string& baseDir, const string& teacherId) {
    string path = joinPath(baseDir, "notification_teacher.csv");
    vector<UserTeacherRecord> all = loadCsv(path);
    vector<TeacherNotification> filtered;

    cout << "\n===== THONG BAO GIAO VIEN =====\n";
    for (size_t i = 0; i < all.size(); ++i) {
        if (getField(all[i].fields, 0) == teacherId) {
            filtered.push_back({i, getField(all[i].fields, 1), getField(all[i].fields, 2), getField(all[i].fields, 3)});
        }
    }

    if (filtered.empty()) {
        cout << "Khong co thong bao.\n"; return;
    }

    for (size_t i = 0; i < filtered.size(); ++i) {
        cout << (i + 1) << ". Phan bo mon \"" << filtered[i].courseName << "\"\n";
    }
    cout << "0. Quay lai\nChon: ";
    int choice;
    if (!(cin >> choice) || choice <= 0 || (size_t)choice > filtered.size()) {
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    size_t idx = filtered[choice - 1].rowIndex;
    all.erase(all.begin() + idx);
    saveCsv(path, all);
    viewTeachingSchedule(baseDir, teacherId);
}

void runTeacherMenu(const string& baseDir, const string& teacherId) {
    int choice;
    while (true) {
        cout << "\n=== MENU GIAO VIEN ===\n";
        cout << "1. Cap nhat thong tin\n";
        cout << "2. Xem lich day & Chi tiet khoa hoc\n";
        cout << "3. Xem danh sach sinh vien\n";
        cout << "4. Nhap diem\n";
        cout << "5. Doi mat khau\n";
        cout << "6. Thong bao\n";
        cout << "0. Dang xuat\n";
        cout << "Chon: ";

        if (!(cin >> choice)) {
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (choice == 0) break;

        switch (choice) {
            case 1: updatePersonalInfo(baseDir, teacherId); break;
            case 2: viewTeachingSchedule(baseDir, teacherId); break;
            case 3: viewStudentList(baseDir, teacherId); break;
            case 4: inputGrades(baseDir, teacherId); break;
            case 5: changePassword(baseDir, teacherId); break;
            case 6: viewTeacherNotifications(baseDir, teacherId); break;
            default: cout << "Lua chon sai!\n"; break;
        }
    }
}
