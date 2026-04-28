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

struct TeacherNotification {
    size_t rowIndex;
    string courseId;
    string courseName;
    string content;
};

vector<TeacherNotification> loadTeacherNotifications(const string& baseDir,
                                                     const string& teacherId,
                                                     vector<UserTeacherRecord>& allNotifications) {
    allNotifications = loadCsv(joinPath(baseDir, "notification_teacher.csv"));
    vector<TeacherNotification> notifications;

    for (size_t i = 0; i < allNotifications.size(); ++i) {
        const vector<string>& fields = allNotifications[i].fields;
        if (getField(fields, 0) == teacherId) {
            notifications.push_back({
                i,
                getField(fields, 1),
                getField(fields, 2),
                getField(fields, 3)
            });
        }
    }

    return notifications;
}

void removeTeacherNotification(const string& baseDir, size_t rowIndex) {
    const string filePath = joinPath(baseDir, "notification_teacher.csv");
    vector<UserTeacherRecord> notifications = loadCsv(filePath);

    if (rowIndex >= notifications.size()) {
        return;
    }

    notifications.erase(notifications.begin() + rowIndex);
    saveCsv(filePath, notifications);
}

void ensureTeacherDateOfBirthField(UserTeacherRecord& record) {
    if (record.fields.size() == 8 && record.fields.back() == "teacher") {
        record.fields.insert(record.fields.begin() + 4, "");
    }
}

void viewTeacherNotifications(const string& baseDir, const string& teacherId) {
    vector<UserTeacherRecord> allNotifications;
    const vector<TeacherNotification> notifications =
        loadTeacherNotifications(baseDir, teacherId, allNotifications);

    cout << "\n===== THONG BAO GIAO VIEN =====\n";

    if (notifications.empty()) {
        cout << "Hien khong co thong bao moi.\n";
        return;
    }

    for (size_t i = 0; i < notifications.size(); ++i) {
        cout << (i + 1) << ". Ban duoc phan bo vao mon \""
             << notifications[i].courseName
             << "\" (thong bao tu Truong)\n";
    }
    cout << "0. Quay lai\n";
    cout << "Chon thong bao de xem: ";

    int choice;
    if (!(cin >> choice)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Lua chon khong hop le.\n";
        return;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (choice <= 0 || static_cast<size_t>(choice) > notifications.size()) {
        return;
    }

    const TeacherNotification notification = notifications[choice - 1];
    cout << "\n" << notification.content << ": "
         << notification.courseName << " (" << notification.courseId << ")\n";
    cout << "Chuyen sang lich day...\n";

    removeTeacherNotification(baseDir, notification.rowIndex);
    viewTeachingSchedule(baseDir, teacherId);
}

void updatePersonalInfo(const string& baseDir, const string& teacherId) {
    string path = joinPath(baseDir, "teacher.csv");
    vector<UserTeacherRecord> teachers = loadCsv(path);

    for (auto& t : teachers) {
        if (!t.fields.empty() && t.fields[0] == teacherId) {
            ensureTeacherDateOfBirthField(t);

            cout << "\n--- CAP NHAT THONG TIN ---\n";
            string in;
            cout << "Ho ten moi: ";
            getline(cin, in);
            if (!trimString(in).empty()) t.fields[1] = trimString(in);

            cout << "Email moi: ";
            getline(cin, in);
            if (!trimString(in).empty()) t.fields[2] = trimString(in);

            cout << "Ngay sinh moi: ";
            getline(cin, in);
            if (!trimString(in).empty()) t.fields[4] = trimString(in);

            cout << "SDT moi: ";
            getline(cin, in);
            if (!trimString(in).empty()) t.fields[5] = trimString(in);

            saveCsv(path, teachers);
            cout << "=> Thanh cong!\n";
            return;
        }
    }
}

void viewTeachingSchedule(const string& baseDir, const string& teacherId) {
    vector<UserTeacherRecord> courses = loadCsv(joinPath(baseDir, "course.csv"));
    cout << "\n--- LICH DAY ---\n";

    bool hasCourse = false;
    for (const auto& c : courses) {
        if (c.fields.size() > 7 && c.fields[3] == teacherId) {
            cout << "- " << c.fields[0] << " | " << c.fields[1]
                 << " | Tu: " << c.fields[6] << " Den: " << c.fields[7] << "\n";
            hasCourse = true;
        }
    }

    if (!hasCourse) cout << "Khong co lich day.\n";
}

void viewStudentList(const string& baseDir, const string& teacherId) {
    string cid;
    cout << "Nhap ma khoa hoc: ";
    getline(cin, cid);
    cid = trimString(cid);

    bool isMyCourse = false;
    vector<UserTeacherRecord> courses = loadCsv(joinPath(baseDir, "course.csv"));
    for (const auto& c : courses) {
        if (c.fields.size() > 3 && c.fields[0] == cid && c.fields[3] == teacherId) {
            isMyCourse = true;
            break;
        }
    }

    if (!isMyCourse) {
        cout << "=> Loi: Ban khong phu trach khoa hoc nay!\n";
        return;
    }

    vector<UserTeacherRecord> enrolls = loadCsv(joinPath(baseDir, "enrollment.csv"));
    vector<UserTeacherRecord> students = loadCsv(joinPath(baseDir, "student.csv"));
    cout << "\n--- DANH SACH SV ---\n";

    bool hasStudent = false;
    for (const auto& e : enrolls) {
        if (e.fields.size() > 2 && e.fields[2] == cid) {
            for (const auto& s : students) {
                if (s.fields.size() > 1 && s.fields[0] == e.fields[1]) {
                    cout << s.fields[0] << " | " << s.fields[1] << "\n";
                    hasStudent = true;
                    break;
                }
            }
        }
    }

    if (!hasStudent) cout << "Chua co sinh vien dang ky.\n";
}

void inputGrades(const string& baseDir, const string& teacherId) {
    string cid, sid, grade;
    cout << "Ma KH: ";
    getline(cin, cid);
    cid = trimString(cid);

    vector<UserTeacherRecord> courses = loadCsv(joinPath(baseDir, "course.csv"));
    bool isMyCourse = false;
    for (const auto& c : courses) {
        if (c.fields.size() > 3 && c.fields[0] == cid && c.fields[3] == teacherId) {
            isMyCourse = true;
            break;
        }
    }

    if (!isMyCourse) {
        cout << "=> Loi: Ban khong phu trach khoa hoc nay!\n";
        return;
    }

    cout << "Ma SV: ";
    getline(cin, sid);
    sid = trimString(sid);
    cout << "Diem: ";
    getline(cin, grade);
    grade = trimString(grade);

    string ePath = joinPath(baseDir, "enrollment.csv");
    vector<UserTeacherRecord> enrolls = loadCsv(ePath);
    bool found = false;
    for (auto& e : enrolls) {
        if (e.fields.size() > 2 && e.fields[1] == sid && e.fields[2] == cid) {
            while (e.fields.size() <= 5) e.fields.push_back("");
            e.fields[5] = grade;
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

void viewCourseList(const string& baseDir, const string& teacherId) {
    vector<UserTeacherRecord> courses = loadCsv(joinPath(baseDir, "course.csv"));
    cout << "\n--- CHI TIET KHOA HOC PHU TRACH ---\n";

    bool found = false;
    for (const auto& c : courses) {
        if (c.fields.size() > 5 && c.fields[3] == teacherId) {
            cout << "- Ma: " << c.fields[0]
                 << " | Ten: " << c.fields[1]
                 << " | So TC: " << c.fields[2]
                 << " | Hoc phi: " << c.fields[5] << "\n";
            found = true;
        }
    }

    if (!found) cout << "Khong tim thay khoa hoc.\n";
}

void changePassword(const string& baseDir, const string& teacherId) {
    string path = joinPath(baseDir, "teacher.csv");
    vector<UserTeacherRecord> teachers = loadCsv(path);

    for (auto& t : teachers) {
        if (!t.fields.empty() && t.fields[0] == teacherId) {
            string oldP, newP;
            cout << "Mat khau hien tai: ";
            getline(cin, oldP);
            if (t.fields[3] != trimString(oldP)) {
                cout << "=> Sai mat khau!\n";
                return;
            }

            cout << "Mat khau moi: ";
            getline(cin, newP);
            if (!trimString(newP).empty()) {
                t.fields[3] = trimString(newP);
                saveCsv(path, teachers);
                cout << "=> Doi mat khau thanh cong!\n";
            }
            return;
        }
    }
}

void runTeacherMenu(const string& baseDir, const string& teacherId) {
    int choice;

    while (true) {
        cout << "\n=== MENU GIAO VIEN ===\n";
        cout << "1. Cap nhat thong tin\n";
        cout << "2. Xem lich day\n";
        cout << "3. Xem danh sach sinh vien\n";
        cout << "4. Nhap diem\n";
        cout << "5. Xem khoa hoc\n";
        cout << "6. Doi mat khau\n";
        cout << "7. Thong bao\n";
        cout << "0. Dang xuat\n";
        cout << "Chon: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 0) break;

        switch (choice) {
            case 1: updatePersonalInfo(baseDir, teacherId); break;
            case 2: viewTeachingSchedule(baseDir, teacherId); break;
            case 3: viewStudentList(baseDir, teacherId); break;
            case 4: inputGrades(baseDir, teacherId); break;
            case 5: viewCourseList(baseDir, teacherId); break;
            case 6: changePassword(baseDir, teacherId); break;
            case 7: viewTeacherNotifications(baseDir, teacherId); break;
            default: cout << "Lua chon sai!\n"; break;
        }
    }
}

void login(const string& baseDir) {
    string user, pass;
    cout << "=== DANG NHAP GIAO VIEN ===\n";
    cout << "ID: ";
    getline(cin, user);
    cout << "Password: ";
    getline(cin, pass);

    auto teachers = loadCsv(joinPath(baseDir, "teacher.csv"));
    bool success = false;
    for (const auto& t : teachers) {
        if (t.fields.size() > 3 && t.fields[0] == user && t.fields[3] == pass) {
            success = true;
            break;
        }
    }

    if (success) {
        cout << "Dang nhap thanh cong!\n";
        runTeacherMenu(baseDir, user);
    } else {
        cout << "Sai tai khoan hoac mat khau!\n";
    }
}
