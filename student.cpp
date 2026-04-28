#include "student.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <ctime>

using namespace std;

namespace {
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
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

string joinPath(const string& dir, const string& fileName) {
    if (dir.empty() || dir == ".") return fileName;
    char last = dir.back();
    if (last == '/' || last == '\\') return dir + fileName;
    return dir + "/" + fileName;
}

string getField(const vector<string>& fields, size_t index) {
    if (index >= fields.size()) return "";
    return fields[index];
}

int toIntSafe(const string& value) {
    try {
        return stoi(trim(value));
    } catch (...) {
        return 0;
    }
}

bool hasDateOfBirth(const vector<string>& fields) {
    return fields.size() >= 10 && fields.back() == "student";
}

size_t studentInfoIndex(size_t newIndex, const vector<string>& fields) {
    if (hasDateOfBirth(fields)) {
        return newIndex;
    }

    if (newIndex >= 4) {
        return newIndex - 1;
    }

    return newIndex;
}

int readChoice() {
    int choice;
    cout << "Nhap lua chon: ";

    if (!(cin >> choice)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return -1;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return choice;
}

void printStudentProfile(const string& baseDir, const string& studentId) {
    vector<UserStudentRecord> students = loadUserList(joinPath(baseDir, "student.csv"));

    for (const UserStudentRecord& student : students) {
        if (getField(student.fields, 0) == studentId) {
            cout << "\n===== THONG TIN SINH VIEN =====\n";
            cout << "ID: " << getField(student.fields, 0) << '\n';
            cout << "Ten: " << getField(student.fields, 1) << '\n';
            cout << "Email: " << getField(student.fields, 2) << '\n';
            cout << "Ngay sinh: " << getField(student.fields, studentInfoIndex(4, student.fields)) << '\n';
            cout << "So dien thoai: " << getField(student.fields, studentInfoIndex(5, student.fields)) << '\n';
            cout << "Dia chi: " << getField(student.fields, studentInfoIndex(6, student.fields)) << '\n';
            cout << "Nganh: " << getField(student.fields, studentInfoIndex(7, student.fields)) << '\n';
            cout << "Lop: " << getField(student.fields, studentInfoIndex(8, student.fields)) << '\n';
            return;
        }
    }

    cout << "Khong tim thay thong tin sinh vien.\n";
}

void printCourseList(const string& baseDir) {
    ifstream file(joinPath(baseDir, "course.csv"));
    if (!file.is_open()) {
        cout << "Khong mo duoc file course.csv!\n";
        return;
    }

    cout << "\n===== DANH SACH KHOA HOC =====\n";
    string line;
    while (getline(file, line)) {
        if (trim(line).empty()) continue;

        vector<string> fields = split(line);
        for (string& field : fields) {
            field = trim(field);
        }

        cout << "Ma: " << getField(fields, 0)
             << " | Ten: " << getField(fields, 1)
             << " | Tin chi: " << getField(fields, 2)
             << " | Hoc phi: " << getField(fields, 5)
             << '\n';
    }
}

void printEnrollmentList(const string& baseDir, const string& studentId) {
    ifstream file(joinPath(baseDir, "enrollment.csv"));
    if (!file.is_open()) {
        cout << "Khong mo duoc file enrollment.csv!\n";
        return;
    }

    cout << "\n===== KHOA HOC DA DANG KY =====\n";
    bool found = false;
    string line;
    while (getline(file, line)) {
        if (trim(line).empty()) continue;

        vector<string> fields = split(line);
        for (string& field : fields) {
            field = trim(field);
        }

        if (getField(fields, 1) == studentId) {
            cout << "Ma dang ky: " << getField(fields, 0)
                 << " | Ma khoa hoc: " << getField(fields, 2)
                 << " | Ngay dang ky: " << getField(fields, 3)
                 << " | Hoc phi: " << getField(fields, 4)
                 << " | Diem: " << getField(fields, 5)
                 << '\n';
            found = true;
        }
    }

    if (!found) {
        cout << "Sinh vien chua dang ky khoa hoc nao.\n";
    }
}

vector<vector<string>> loadPipeRows(const string& filePath) {
    vector<vector<string>> rows;
    ifstream file(filePath);

    if (!file.is_open()) {
        cout << "Khong mo duoc file " << filePath << "!\n";
        return rows;
    }

    string line;
    while (getline(file, line)) {
        if (trim(line).empty()) continue;

        vector<string> fields = split(line);
        for (string& field : fields) {
            field = trim(field);
        }
        rows.push_back(fields);
    }

    return rows;
}

bool isStudentEnrolled(const vector<vector<string>>& enrollments,
                       const string& studentId,
                       const string& courseId) {
    for (const vector<string>& enrollment : enrollments) {
        if (getField(enrollment, 1) == studentId && getField(enrollment, 2) == courseId) {
            return true;
        }
    }

    return false;
}

int countCourseEnrollments(const vector<vector<string>>& enrollments, const string& courseId) {
    int count = 0;

    for (const vector<string>& enrollment : enrollments) {
        if (getField(enrollment, 2) == courseId) {
            ++count;
        }
    }

    return count;
}

string findCourseName(const vector<vector<string>>& courses, const string& courseId) {
    for (const vector<string>& course : courses) {
        if (getField(course, 0) == courseId) {
            return getField(course, 1);
        }
    }

    return courseId;
}

string todayDate() {
    time_t now = time(nullptr);
    tm localTm{};
#ifdef _MSC_VER
    localtime_s(&localTm, &now);
#else
    localTm = *localtime(&now);
#endif
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%d-%m-%Y", &localTm);
    return string(buffer);
}

string nextEnrollmentId(const vector<vector<string>>& enrollments) {
    int maxNumber = 0;

    for (const vector<string>& enrollment : enrollments) {
        const string id = getField(enrollment, 0);
        if (id.size() > 2 && id[0] == 'E' && id[1] == 'N') {
            maxNumber = max(maxNumber, toIntSafe(id.substr(2)));
        }
    }

    stringstream ss;
    ss << "EN";
    if (maxNumber + 1 < 10) ss << "00";
    else if (maxNumber + 1 < 100) ss << "0";
    ss << (maxNumber + 1);
    return ss.str();
}

bool appendPipeRow(const string& filePath, const vector<string>& fields) {
    ofstream file(filePath, ios::app);
    if (!file.is_open()) {
        cout << "Khong ghi duoc file " << filePath << "!\n";
        return false;
    }

    for (size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) file << " | ";
        file << fields[i];
    }
    file << '\n';
    return true;
}

void showCourseDetail(const vector<string>& course, int registeredCount) {
    const int maxStudents = toIntSafe(getField(course, 4));

    cout << "\n===== CHI TIET MON HOC =====\n";
    cout << "Ma mon: " << getField(course, 0) << '\n';
    cout << "Ten mon: " << getField(course, 1) << '\n';
    cout << "So tin chi: " << getField(course, 2) << '\n';
    cout << "Ma giao vien: " << getField(course, 3) << '\n';
    cout << "Con trong: " << (maxStudents - registeredCount) << "/" << maxStudents << '\n';
    cout << "Tien hoc moi tin chi: " << getField(course, 5) << '\n';
    cout << "Ngay bat dau: " << getField(course, 6) << '\n';
    cout << "Ngay ket thuc: " << getField(course, 7) << '\n';
}

void registerCourseFromNotification(const string& baseDir,
                                    const string& studentId,
                                    const vector<string>& course) {
    const string enrollmentPath = joinPath(baseDir, "enrollment.csv");
    vector<vector<string>> enrollments = loadPipeRows(enrollmentPath);
    const string courseId = getField(course, 0);
    const int maxStudents = toIntSafe(getField(course, 4));
    const int registeredCount = countCourseEnrollments(enrollments, courseId);

    showCourseDetail(course, registeredCount);

    if (isStudentEnrolled(enrollments, studentId, courseId)) {
        cout << "Ban da dang ky mon hoc nay.\n";
        return;
    }

    if (maxStudents <= 0 || registeredCount >= maxStudents) {
        cout << "Mon hoc da dong hoac khong con cho trong.\n";
        return;
    }

    cout << "\n1. Dang ky mon hoc nay\n";
    cout << "0. Quay lai\n";

    if (readChoice() != 1) {
        return;
    }

    const int credits = toIntSafe(getField(course, 2));
    const int feePerCredit = toIntSafe(getField(course, 5));
    const int totalFee = credits * feePerCredit;

    if (appendPipeRow(enrollmentPath, {
            nextEnrollmentId(enrollments),
            studentId,
            courseId,
            todayDate(),
            "Registered",
            "",
            "",
            to_string(totalFee),
            "Unpaid"
        })) {
        cout << "Dang ky mon hoc thanh cong.\n";
    }
}

void showGradeDetail(const string& baseDir, const string& studentId, const string& courseId) {
    const vector<vector<string>> courses = loadPipeRows(joinPath(baseDir, "course.csv"));
    const vector<vector<string>> enrollments = loadPipeRows(joinPath(baseDir, "enrollment.csv"));

    cout << "\n===== THONG TIN DIEM MON HOC =====\n";
    for (const vector<string>& enrollment : enrollments) {
        if (getField(enrollment, 1) == studentId && getField(enrollment, 2) == courseId) {
            cout << "Mon hoc: " << findCourseName(courses, courseId) << " (" << courseId << ")\n";
            cout << "Ngay dang ky: " << getField(enrollment, 3) << '\n';
            cout << "Trang thai dang ky: " << getField(enrollment, 4) << '\n';
            cout << "Diem: " << getField(enrollment, 5) << '\n';
            return;
        }
    }

    cout << "Khong tim thay thong tin mon hoc.\n";
}

struct StudentNotification {
    int type;
    string courseId;
    string courseName;
    vector<string> course;
};

vector<StudentNotification> buildNotifications(const string& baseDir, const string& studentId) {
    const vector<vector<string>> courses = loadPipeRows(joinPath(baseDir, "course.csv"));
    const vector<vector<string>> enrollments = loadPipeRows(joinPath(baseDir, "enrollment.csv"));
    vector<StudentNotification> notifications;

    for (const vector<string>& course : courses) {
        const string courseId = getField(course, 0);
        const int maxStudents = toIntSafe(getField(course, 4));
        const int registeredCount = countCourseEnrollments(enrollments, courseId);

        if (maxStudents > 0 &&
            registeredCount < maxStudents &&
            !isStudentEnrolled(enrollments, studentId, courseId)) {
            notifications.push_back({1, courseId, getField(course, 1), course});
        }
    }

    for (const vector<string>& enrollment : enrollments) {
        if (getField(enrollment, 1) != studentId) {
            continue;
        }

        const string grade = trim(getField(enrollment, 5));
        if (!grade.empty() && grade != "0") {
            const string courseId = getField(enrollment, 2);
            notifications.push_back({2, courseId, findCourseName(courses, courseId), {}});
        }
    }

    return notifications;
}

void showNotifications(const string& baseDir, const string& studentId) {
    const vector<StudentNotification> notifications = buildNotifications(baseDir, studentId);

    cout << "\n===== THONG BAO =====\n";
    if (notifications.empty()) {
        cout << "Hien chua co thong bao moi.\n";
        return;
    }

    for (size_t i = 0; i < notifications.size(); ++i) {
        cout << (i + 1) << ". ";
        if (notifications[i].type == 1) {
            cout << "Ban co mon \"" << notifications[i].courseName
                 << "\" chua dang ky hien dang mo (thong bao tu Truong)\n";
        } else {
            cout << "Ban co thong bao diem tu mon \"" << notifications[i].courseName
                 << "\" (Thong bao tu giao vien)\n";
        }
    }
    cout << "0. Quay lai\n";

    const int choice = readChoice();
    if (choice <= 0 || static_cast<size_t>(choice) > notifications.size()) {
        return;
    }

    const StudentNotification& notification = notifications[choice - 1];
    if (notification.type == 1) {
        registerCourseFromNotification(baseDir, studentId, notification.course);
    } else {
        showGradeDetail(baseDir, studentId, notification.courseId);
    }
}

bool saveStudentList(const string& filePath, const vector<UserStudentRecord>& students) {
    ofstream file(filePath);
    if (!file.is_open()) {
        cout << "Khong ghi duoc file " << filePath << "!\n";
        return false;
    }

    for (const UserStudentRecord& student : students) {
        for (size_t i = 0; i < student.fields.size(); ++i) {
            if (i > 0) {
                file << " | ";
            }
            file << student.fields[i];
        }
        file << '\n';
    }

    return true;
}

void changeStudentPassword(const string& baseDir, const string& studentId) {
    const string filePath = joinPath(baseDir, "student.csv");
    vector<UserStudentRecord> students = loadUserList(filePath);

    for (UserStudentRecord& student : students) {
        if (getField(student.fields, 0) == studentId) {
            string oldPassword;
            string newPassword;
            string confirmPassword;

            cout << "Mat khau hien tai: ";
            getline(cin, oldPassword);
            if (getField(student.fields, 3) != oldPassword) {
                cout << "Mat khau hien tai khong dung.\n";
                return;
            }

            cout << "Mat khau moi: ";
            getline(cin, newPassword);
            newPassword = trim(newPassword);
            if (newPassword.empty()) {
                cout << "Mat khau moi khong duoc de trong.\n";
                return;
            }

            cout << "Nhap lai mat khau moi: ";
            getline(cin, confirmPassword);
            if (newPassword != trim(confirmPassword)) {
                cout << "Mat khau xac nhan khong khop.\n";
                return;
            }

            student.fields[3] = newPassword;
            if (saveStudentList(filePath, students)) {
                cout << "Doi mat khau thanh cong.\n";
            }
            return;
        }
    }

    cout << "Khong tim thay thong tin sinh vien.\n";
}
}


vector<UserStudentRecord> loadUserList(const string& filePath) {
    vector<UserStudentRecord> users;
    ifstream file(filePath);
    string line;

    if (!file.is_open()) {
        cout << "Khong mo duoc file!\n";
        return users;
    }

    while (getline(file, line)) {
        if (line.empty()) continue;

        vector<string> fields = split(line);
        for (auto& field : fields) {
            field = trim(field);
        }

        if (fields.size() < 9) continue;

        UserStudentRecord user;
        user.fields = std::move(fields);
        users.push_back(user);
    }

    return users;
}

void runStudentMenu(const string& baseDir, const string& studentId) {
    while (true) {
        cout << "\n========== MENU SINH VIEN ==========\n";
        cout << "1. Xem thong tin ca nhan\n";
        cout << "2. Xem danh sach khoa hoc\n";
        cout << "3. Xem khoa hoc da dang ky\n";
        cout << "4. Doi mat khau\n";
        cout << "5. Thong bao\n";
        cout << "0. Dang xuat\n";

        switch (readChoice()) {
            case 1:
                printStudentProfile(baseDir, studentId);
                break;
            case 2:
                printCourseList(baseDir);
                break;
            case 3:
                printEnrollmentList(baseDir, studentId);
                break;
            case 4:
                changeStudentPassword(baseDir, studentId);
                break;
            case 5:
                showNotifications(baseDir, studentId);
                break;
            case 0:
                cout << "Dang xuat...\n";
                return;
            default:
                cout << "Lua chon khong hop le.\n";
                break;
        }
    }
}

