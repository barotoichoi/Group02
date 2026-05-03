#include "teacher.h"
#include <cstdlib>
#include <fstream>
#include <iomanip>
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

int readMenuChoice() {
    int choice;
    cout << "Choose: ";

    if (!(cin >> choice)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return -1;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return choice;
}

void exitProgram() {
    cout << "Program exited.\n";
    exit(0);
}

bool hasTeacherDateOfBirth(const vector<string>& fields) {
    return fields.size() >= 9 && fields.back() == "teacher";
}

size_t teacherPhoneIndex(const vector<string>& fields) {
    return 4;
}

size_t teacherDateOfBirthIndex(const vector<string>& fields) {
    return hasTeacherDateOfBirth(fields) ? 5 : fields.size();
}

size_t teacherAddressIndex(const vector<string>& fields) {
    return hasTeacherDateOfBirth(fields) ? 6 : 5;
}

size_t teacherDepartmentIndex(const vector<string>& fields) {
    return hasTeacherDateOfBirth(fields) ? 7 : 6;
}

bool hasStudentDateOfBirth(const vector<string>& fields) {
    return fields.size() >= 10 && fields.back() == "student";
}

size_t studentInfoIndex(const vector<string>& fields, size_t newIndex) {
    if (hasStudentDateOfBirth(fields)) return newIndex;
    if (newIndex >= 4) return newIndex - 1;
    return newIndex;
}

string resultFromScore(const string& scoreText) {
    const string score = trimString(scoreText);
    if (score.empty()) return "";

    try {
        return stod(score) >= 5.0 ? "Passed" : "Failed";
    } catch (...) {
        return "";
    }
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
    cout << "\n===== TEACHING SCHEDULE =====\n";

    const int w1 = 12, w2 = 30, w3 = 9, w4 = 14, w5 = 14, w6 = 14;
    cout << left << setw(w1) << "Course ID"
         << setw(w2) << "Course Name"
         << setw(w3) << "Credits"
         << setw(w4) << "Tuition"
         << setw(w5) << "Start Date"
         << setw(w6) << "End Date" << '\n';
    cout << string(w1 + w2 + w3 + w4 + w5 + w6, '-') << '\n';

    bool hasCourse = false;
    for (const auto& c : courses) {
        if (c.fields.size() > 7 && c.fields[3] == teacherId) {
            if (trimString(c.fields[4]) == "pending") continue;

            cout << left << setw(w1) << getField(c.fields, 0)
                 << setw(w2) << getField(c.fields, 1)
                 << setw(w3) << getField(c.fields, 2)
                 << setw(w4) << getField(c.fields, 5)
                 << setw(w5) << getField(c.fields, 6)
                 << setw(w6) << getField(c.fields, 7) << '\n';
            hasCourse = true;
        }
    }
    if (!hasCourse) cout << "No valid teaching schedule.\n";
}

void inputGrades(const string& baseDir, const string& teacherId, const string& courseId) {
    vector<UserTeacherRecord> courses = loadCsv(joinPath(baseDir, "course.csv"));
    bool canInput = false;
    for (const auto& c : courses) {
        if (c.fields.size() > 4 && c.fields[0] == courseId && c.fields[3] == teacherId) {
            if (trimString(c.fields[4]) == "pending") {
                cout << "=> Error: this course is pending, so grades cannot be entered!\n";
                return;
            }
            canInput = true;
            break;
        }
    }

    if (!canInput) {
        cout << "=> Error: you are not assigned to this course!\n";
        return;
    }

    string ePath = joinPath(baseDir, "enrollment.csv");
    vector<UserTeacherRecord> enrollments = loadCsv(ePath);
    vector<UserTeacherRecord> students = loadCsv(joinPath(baseDir, "student.csv"));
    vector<size_t> enrollmentIndexes;

    for (size_t i = 0; i < enrollments.size(); ++i) {
        if (getField(enrollments[i].fields, 2) == courseId) {
            enrollmentIndexes.push_back(i);
        }
    }

    if (enrollmentIndexes.empty()) {
        cout << "No students enrolled yet.\n";
        return;
    }

    while (true) {
        cout << "\n===== ADD GRADES =====\n";
        for (size_t i = 0; i < enrollmentIndexes.size(); ++i) {
            const UserTeacherRecord& enrollment = enrollments[enrollmentIndexes[i]];
            string studentName = "Unknown";

            for (const auto& student : students) {
                if (getField(student.fields, 0) == getField(enrollment.fields, 1)) {
                    studentName = getField(student.fields, 1);
                    break;
                }
            }

            cout << (i + 1) << ". " << getField(enrollment.fields, 1)
                 << " - " << studentName
                 << " | Current score: " << getField(enrollment.fields, 5) << '\n';
        }
        cout << "0. Save and exit\n";
        cout << "9. Exit program\n";

        const int choice = readMenuChoice();
        if (choice == 0) {
            saveCsv(ePath, enrollments);
            cout << "=> Grades saved successfully!\n";
            return;
        }
        if (choice == 9) {
            saveCsv(ePath, enrollments);
            exitProgram();
        }
        if (choice < 1 || static_cast<size_t>(choice) > enrollmentIndexes.size()) {
            cout << "Invalid choice.\n";
            continue;
        }

        string grade;
        cout << "Enter score (leave blank to keep empty): ";
        getline(cin, grade);

        UserTeacherRecord& enrollment = enrollments[enrollmentIndexes[choice - 1]];
        while (enrollment.fields.size() <= 6) {
            enrollment.fields.push_back("");
        }
        enrollment.fields[5] = trimString(grade);
        enrollment.fields[6] = resultFromScore(grade);
    }
}

void inputGrades(const string& baseDir, const string& teacherId) {
    string courseId;
    cout << "Course ID: ";
    getline(cin, courseId);
    inputGrades(baseDir, teacherId, trimString(courseId));
}

void updatePersonalInfo(const string& baseDir, const string& teacherId) {
    string path = joinPath(baseDir, "teacher.csv");
    vector<UserTeacherRecord> teachers = loadCsv(path);
    for (auto& t : teachers) {
        if (!t.fields.empty() && t.fields[0] == teacherId) {
            if (t.fields.size() == 8 && t.fields.back() == "teacher") {
                t.fields.insert(t.fields.begin() + 5, "");
            }
            string in;
            cout << "\n--- UPDATE INFORMATION ---\n";
            cout << "New full name: "; getline(cin, in);
            if (!trimString(in).empty()) t.fields[1] = trimString(in);
            cout << "New email: "; getline(cin, in);
            if (!trimString(in).empty()) t.fields[2] = trimString(in);
            cout << "New date of birth: "; getline(cin, in);
            if (!trimString(in).empty()) t.fields[teacherDateOfBirthIndex(t.fields)] = trimString(in);
            cout << "New phone: "; getline(cin, in);
            if (!trimString(in).empty()) t.fields[teacherPhoneIndex(t.fields)] = trimString(in);
            saveCsv(path, teachers);
            cout << "=> Success!\n";
            return;
        }
    }
}

void viewTeacherProfile(const string& baseDir, const string& teacherId) {
    vector<UserTeacherRecord> teachers = loadCsv(joinPath(baseDir, "teacher.csv"));

    for (const auto& teacher : teachers) {
        if (getField(teacher.fields, 0) == teacherId) {
            cout << "\n===== TEACHER INFORMATION =====\n";
            cout << "ID: " << getField(teacher.fields, 0) << '\n';
            cout << "Full name: " << getField(teacher.fields, 1) << '\n';
            cout << "Email: " << getField(teacher.fields, 2) << '\n';
            cout << "Date of birth: " << getField(teacher.fields, teacherDateOfBirthIndex(teacher.fields)) << '\n';
            cout << "Phone: " << getField(teacher.fields, teacherPhoneIndex(teacher.fields)) << '\n';
            cout << "Address: " << getField(teacher.fields, teacherAddressIndex(teacher.fields)) << '\n';
            cout << "Department: " << getField(teacher.fields, teacherDepartmentIndex(teacher.fields)) << '\n';
            return;
        }
    }

    cout << "Teacher information not found.\n";
}

void personalInformationMenu(const string& baseDir, const string& teacherId) {
    while (true) {
        cout << "\n===== PERSONAL INFORMATION =====\n";
        cout << "1. View profile\n";
        cout << "2. Update profile\n";
        cout << "0. Back\n";

        switch (readMenuChoice()) {
            case 1:
                viewTeacherProfile(baseDir, teacherId);
                break;
            case 2:
                updatePersonalInfo(baseDir, teacherId);
                break;
            case 0:
                return;
            default:
                cout << "Invalid choice.\n";
                break;
        }
    }
}

void viewStudentList(const string& baseDir, const string& teacherId) {
    vector<UserTeacherRecord> courses = loadCsv(joinPath(baseDir, "course.csv"));
    vector<UserTeacherRecord> teacherCourses;

    for (const auto& course : courses) {
        if (getField(course.fields, 3) == teacherId) {
            teacherCourses.push_back(course);
        }
    }

    if (teacherCourses.empty()) {
        cout << "No assigned courses.\n";
        return;
    }

    while (true) {
        cout << "\n===== COURSE STUDENT LISTS =====\n";
        for (size_t i = 0; i < teacherCourses.size(); ++i) {
            cout << (i + 1) << ". " << getField(teacherCourses[i].fields, 0)
                 << " - " << getField(teacherCourses[i].fields, 1) << '\n';
        }
        cout << "0. Back\n";
        cout << "9. Exit program\n";

        const int choice = readMenuChoice();
        if (choice == 0) return;
        if (choice == 9) exitProgram();
        if (choice < 1 || static_cast<size_t>(choice) > teacherCourses.size()) {
            cout << "Invalid choice.\n";
            continue;
        }

        const string courseId = getField(teacherCourses[choice - 1].fields, 0);
        while (true) {
            vector<UserTeacherRecord> enrollments = loadCsv(joinPath(baseDir, "enrollment.csv"));
            vector<UserTeacherRecord> students = loadCsv(joinPath(baseDir, "student.csv"));

            cout << "\n===== " << getField(teacherCourses[choice - 1].fields, 1)
                 << " (" << courseId << ") =====\n";
            const int w1 = 5, w2 = 13, w3 = 24, w4 = 14, w5 = 12, w6 = 10, w7 = 12;
            cout << left << setw(w1) << "No."
                 << setw(w2) << "Student ID"
                 << setw(w3) << "Full name"
                 << setw(w4) << "Class"
                 << setw(w5) << "Status"
                 << setw(w6) << "Score"
                 << setw(w7) << "Result" << '\n';
            cout << string(w1 + w2 + w3 + w4 + w5 + w6 + w7, '-') << '\n';

            bool hasStudent = false;
            int no = 1;
            for (const auto& enrollment : enrollments) {
                if (getField(enrollment.fields, 2) != courseId) continue;

                string studentName = "Unknown";
                string studentClass = "";
                for (const auto& student : students) {
                    if (getField(student.fields, 0) == getField(enrollment.fields, 1)) {
                        studentName = getField(student.fields, 1);
                        studentClass = getField(student.fields, studentInfoIndex(student.fields, 8));
                        break;
                    }
                }

                cout << left << setw(w1) << no
                     << setw(w2) << getField(enrollment.fields, 1)
                     << setw(w3) << studentName
                     << setw(w4) << studentClass
                     << setw(w5) << getField(enrollment.fields, 4)
                     << setw(w6) << getField(enrollment.fields, 5)
                     << setw(w7) << getField(enrollment.fields, 6) << '\n';
                ++no;
                hasStudent = true;
            }

            if (!hasStudent) {
                cout << "No students enrolled yet.\n";
            }

            cout << "\n1. Add grades\n";
            cout << "0. Back\n";
            cout << "9. Exit program\n";

            const int tableChoice = readMenuChoice();
            if (tableChoice == 0) break;
            if (tableChoice == 9) exitProgram();
            if (tableChoice == 1) {
                inputGrades(baseDir, teacherId, courseId);
            } else {
                cout << "Invalid choice.\n";
            }
        }
    }
}

void changePassword(const string& baseDir, const string& teacherId) {
    string path = joinPath(baseDir, "teacher.csv");
    vector<UserTeacherRecord> teachers = loadCsv(path);
    for (auto& t : teachers) {
        if (!t.fields.empty() && t.fields[0] == teacherId) {
            string oldP, newP;
            cout << "Current password: "; getline(cin, oldP);
            if (t.fields[3] != trimString(oldP)) {
                cout << "=> Incorrect password!\n"; return;
            }
            cout << "New password: "; getline(cin, newP);
            if (!trimString(newP).empty()) {
                t.fields[3] = trimString(newP);
                saveCsv(path, teachers);
                cout << "=> Password changed successfully!\n";
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

    cout << "\n===== TEACHER NOTIFICATIONS =====\n";
    for (size_t i = 0; i < all.size(); ++i) {
        if (getField(all[i].fields, 0) == teacherId) {
            filtered.push_back({i, getField(all[i].fields, 1), getField(all[i].fields, 2), getField(all[i].fields, 3)});
        }
    }

    if (filtered.empty()) {
        cout << "No notifications.\n"; return;
    }

    for (size_t i = 0; i < filtered.size(); ++i) {
        cout << (i + 1) << ". Assigned course \"" << filtered[i].courseName << "\"\n";
    }
    cout << "0. Back\nChoose: ";
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
        cout << "\n=== TEACHER MENU ===\n";
        cout << "1. Personal information\n";
        cout << "2. View teaching schedule and course details\n";
        cout << "3. View student list\n";
        cout << "4. Change password\n";
        cout << "5. Notifications\n";
        cout << "0. Log out\n";
        cout << "Choose: ";

        if (!(cin >> choice)) {
            cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (choice == 0) break;

        switch (choice) {
            case 1: personalInformationMenu(baseDir, teacherId); break;
            case 2: viewTeachingSchedule(baseDir, teacherId); break;
            case 3: viewStudentList(baseDir, teacherId); break;
            case 4: changePassword(baseDir, teacherId); break;
            case 5: viewTeacherNotifications(baseDir, teacherId); break;
            default: cout << "Invalid choice!\n"; break;
        }
    }
}
