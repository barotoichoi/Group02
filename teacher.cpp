#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <vector>
#include <string>

using namespace std;

struct CsvRecord {
    vector<string> fields;
};

string trimString(const string& value) {
    const size_t first = value.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    const size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

vector<CsvRecord> loadCsv(const string& filePath) {
    vector<CsvRecord> records;
    ifstream fin(filePath);
    if (!fin.is_open()) return records;
    
    string line;
    bool firstLine = true;
    while (getline(fin, line)) {
        if (firstLine) {
            if (line.size() >= 3 && line[0] == (char)0xEF && line[1] == (char)0xBB && line[2] == (char)0xBF) {
                line = line.substr(3);
            }
            firstLine = false;
        }
        
        if (trimString(line).empty()) continue;
        CsvRecord record;
        stringstream ss(line);
        string token;
        while (getline(ss, token, '|')) {
            record.fields.push_back(trimString(token));
        }
        records.push_back(record);
    }
    return records;
}

void saveCsv(const string& filePath, const vector<CsvRecord>& records) {
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

void updatePersonalInfo(const string& teacherId) {
    vector<CsvRecord> teachers = loadCsv("teacher.csv");
    for (auto& t : teachers) {
        if (!t.fields.empty() && t.fields[0] == teacherId) {
            cout << "\n--- CAP NHAT THONG TIN ---\n";
            string in;
            cout << "Ho ten moi: "; getline(cin, in); if(!trimString(in).empty()) t.fields[1] = trimString(in);
            cout << "Email moi: "; getline(cin, in); if(!trimString(in).empty()) t.fields[2] = trimString(in);
            cout << "SDT moi: "; getline(cin, in); if(!trimString(in).empty()) t.fields[4] = trimString(in);
            saveCsv("teacher.csv", teachers);
            cout << "=> Thanh cong!\n";
            return;
        }
    }
}

void changePassword(const string& teacherId) {
    vector<CsvRecord> teachers = loadCsv("teacher.csv");
    for (auto& t : teachers) {
        if (!t.fields.empty() && t.fields[0] == teacherId) {
            string oldP, newP;
            cout << "Mat khau hien tai: "; getline(cin, oldP);
            if (t.fields[3] != trimString(oldP)) { cout << "=> Sai mat khau!\n"; return; }
            cout << "Mat khau moi: "; getline(cin, newP);
            if (!trimString(newP).empty()) {
                t.fields[3] = trimString(newP);
                saveCsv("teacher.csv", teachers);
                cout << "=> Doi mat khau thanh cong!\n";
            } else {
                cout << "=> Mat khau khong duoc de trong!\n";
            }
            return;
        }
    }
}

void viewTeachingSchedule(const string& teacherId) {
    vector<CsvRecord> courses = loadCsv("course.csv");
    cout << "\n--- LICH DAY ---\n";
    bool hasCourse = false;
    for (const auto& c : courses) {
        if (c.fields.size() > 3 && c.fields[3] == teacherId) {
            cout << "- Ma KH: " << c.fields[0] << " | Ten KH: " << c.fields[1] << " | Tgian: " << (c.fields.size() > 6 ? c.fields[6] : "N/A") << "\n";
            hasCourse = true;
        }
    }
    if (!hasCourse) cout << "Hien tai chua co lich day.\n";
}

void viewStudentList(const string& teacherId) {
    string cid; 
    cout << "Nhap ma khoa hoc: "; 
    getline(cin, cid);
    cid = trimString(cid);

    bool isMyCourse = false;
    vector<CsvRecord> courses = loadCsv("course.csv");
    for (const auto& c : courses) {
        if (c.fields.size() > 3 && c.fields[0] == cid && c.fields[3] == teacherId) {
            isMyCourse = true;
            break;
        }
    }

    if (!isMyCourse) {
        cout << "=> Loi: Ma khoa hoc khong hop le hoac ban khong phu trach khoa hoc nay!\n";
        return;
    }

    vector<CsvRecord> enrolls = loadCsv("enrollment.csv");
    vector<CsvRecord> students = loadCsv("student.csv");
    cout << "\n--- DANH SACH SV ---\n";
    
    bool hasStudent = false;
    vector<string> printedStudentIds; 

    for (const auto& e : enrolls) {
        if (e.fields.size() > 2 && e.fields[2] == cid) {
            string sid = e.fields[1];
            
            if (find(printedStudentIds.begin(), printedStudentIds.end(), sid) != printedStudentIds.end()) {
                continue; 
            }
            
            for(const auto& s : students) {
                if(s.fields.size() > 1 && s.fields[0] == sid) {
                    cout << s.fields[0] << " | " << s.fields[1] << "\n";
                    printedStudentIds.push_back(sid);
                    hasStudent = true;
                    break;
                }
            }
        }
    }
    
    if (!hasStudent) cout << "Chua co sinh vien nao dang ky khoa hoc nay.\n";
}

void inputGrades(const string& teacherId) {
    string cid, sid, grade;
    cout << "Ma KH: "; getline(cin, cid);
    cid = trimString(cid);
    
    bool isMyCourse = false;
    vector<CsvRecord> courses = loadCsv("course.csv");
    for (const auto& c : courses) {
        if (c.fields.size() > 3 && c.fields[0] == cid && c.fields[3] == teacherId) {
            isMyCourse = true;
            break;
        }
    }

    if (!isMyCourse) {
        cout << "=> Loi: Ma khoa hoc khong hop le hoac ban khong phu trach khoa hoc nay!\n";
        return;
    }

    cout << "Ma SV: "; getline(cin, sid); sid = trimString(sid);
    cout << "Diem: "; getline(cin, grade); grade = trimString(grade);
    
    vector<CsvRecord> enrolls = loadCsv("enrollment.csv");
    bool found = false;
    for (auto& e : enrolls) {
        if (e.fields.size() > 2 && e.fields[1] == sid && e.fields[2] == cid) {
            while(e.fields.size() <= 5) e.fields.push_back("");
            e.fields[5] = grade;
            found = true;
        }
    }
    
    if (found) {
        saveCsv("enrollment.csv", enrolls);
        cout << "=> Da luu diem thanh cong!\n"; 
    } else {
        cout << "=> Loi: Khong tim thay sinh vien hoac sinh vien chua dang ky khoa hoc nay!\n";
    }
}

void viewNotifications() {
    cout << "\n--- THONG BAO ---\n1. Han nop diem: 15/05\n2. Hop chuyen mon: Thu 2\n";
}

void runTeacherMenu(const string& teacherId) {
    int choice;
    while (true) {
        cout << "\n1.TT Ca nhan | 2.Doi MK | 3.Lich day | 4.DS Sinh vien | 5.Nhap diem | 6.Thong bao | 0.Thoat\nChon: ";
        if(!(cin >> choice)) { 
            cin.clear(); 
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
            continue; 
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        if (choice == 0) break;
        switch(choice) {
            case 1: updatePersonalInfo(teacherId); break;
            case 2: changePassword(teacherId); break;
            case 3: viewTeachingSchedule(teacherId); break;
            case 4: viewStudentList(teacherId); break;
            case 5: inputGrades(teacherId); break;
            case 6: viewNotifications(); break;
            default: cout << "Lua chon khong hop le!\n"; break;
        }
    }
}

int main() {
    string user, pass;
    cout << "=== DANG NHAP GIAO VIEN ===\n";
    cout << "ID: "; getline(cin, user);
    cout << "Password: "; getline(cin, pass);

    auto teachers = loadCsv("teacher.csv");
    bool success = false;
    for (const auto& t : teachers) {
        if (t.fields.size() > 3 && t.fields[0] == user && t.fields[3] == pass) {
            success = true; break;
        }
    }

    if (success) {
        cout << "Dang nhap thanh cong!\n";
        runTeacherMenu(user);
    } else {
        cout << "Sai tai khoan hoac mat khau!\n";
    }
    return 0;
}
