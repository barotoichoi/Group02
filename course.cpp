#include <cctype>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const string BASE_PATH = "";
const string COURSE_FILE = BASE_PATH + "course.csv";
const string ENROLL_FILE = BASE_PATH + "enrollment.csv";
const string STUDENT_FILE = BASE_PATH + "student.csv";
const string TEACHER_FILE = BASE_PATH + "teacher.csv";

namespace {

static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static vector<string> splitByPipe(const string& line) {
    vector<string> cols;
    string token;
    stringstream ss(line);

    while (getline(ss, token, '|')) {
        cols.push_back(trim(token));
    }
    return cols;
}

static vector<vector<string>> readCsvRows(const string& filePath) {
    vector<vector<string>> rows;
    ifstream fin(filePath);

    if (!fin.is_open()) {
        cout << "Khong mo duoc file: " << filePath << "\n";
        return rows;
    }

    string line;
    bool firstLine = true;
    while (getline(fin, line)) {
        if (firstLine && line.size() >= 3 &&
            static_cast<unsigned char>(line[0]) == 0xEF &&
            static_cast<unsigned char>(line[1]) == 0xBB &&
            static_cast<unsigned char>(line[2]) == 0xBF) {
            line = line.substr(3);
        }
        firstLine = false;

        if (trim(line).empty()) continue;
        rows.push_back(splitByPipe(line));
    }

    return rows;
}

static bool writeCsvRows(const string& filePath, const vector<vector<string>>& rows) {
    ofstream fout(filePath);
    if (!fout.is_open()) {
        cout << "Khong ghi duoc file: " << filePath << "\n";
        return false;
    }

    for (size_t r = 0; r < rows.size(); ++r) {
        for (size_t c = 0; c < rows[r].size(); ++c) {
            if (c > 0) fout << " | ";
            fout << rows[r][c];
        }
        fout << "\n";
    }
    return true;
}

static string getCell(const vector<string>& row, size_t index) {
    if (index >= row.size()) return "";
    return row[index];
}

static string todayDDMMYYYY() {
    time_t now = time(nullptr);
    tm localTm{};
#ifdef _WIN32
#ifdef _MSC_VER
    localtime_s(&localTm, &now);
#else
    localTm = *localtime(&now);
#endif
#else
    localtime_r(&now, &localTm);
#endif
    char buffer[16];
    strftime(buffer, sizeof(buffer), "%d-%m-%Y", &localTm);
    return string(buffer);
}

static int readChoice() {
    int choice;
    cout << "Nhap lua chon (1-7): ";

    if (!(cin >> choice)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return -1;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return choice;
}

static vector<vector<string>> readCourseFile() {
    return readCsvRows(COURSE_FILE);
}

static string findCourseName(const string& courseId, const vector<vector<string>>& courses) {
    for (size_t i = 0; i < courses.size(); ++i) {
        if (getCell(courses[i], 0) == courseId) {
            return getCell(courses[i], 1);
        }
    }
    return "Unknown";
}

static bool findCourseRow(const string& courseId, const vector<vector<string>>& courses, vector<string>& outRow) {
    for (size_t i = 0; i < courses.size(); ++i) {
        if (getCell(courses[i], 0) == courseId) {
            outRow = courses[i];
            return true;
        }
    }
    return false;
}

static bool enrollmentExists(const string& studentId, const string& courseId,
                             const vector<vector<string>>& enrollments) {
    for (size_t i = 0; i < enrollments.size(); ++i) {
        if (getCell(enrollments[i], 1) == studentId && getCell(enrollments[i], 2) == courseId) {
            return true;
        }
    }
    return false;
}

static string nextEnrollmentId(const vector<vector<string>>& enrollments) {
    int maxNumber = 0;

    for (size_t i = 0; i < enrollments.size(); ++i) {
        string id = getCell(enrollments[i], 0);
        if (id.size() < 3 || id.substr(0, 2) != "EN") continue;

        string numberPart = id.substr(2);
        bool isAllDigits = !numberPart.empty();
        for (size_t j = 0; j < numberPart.size(); ++j) {
            if (!isdigit(static_cast<unsigned char>(numberPart[j]))) {
                isAllDigits = false;
                break;
            }
        }

        if (isAllDigits) {
            int n = stoi(numberPart);
            if (n > maxNumber) maxNumber = n;
        }
    }

    stringstream ss;
    ss << "EN" << setw(3) << setfill('0') << (maxNumber + 1);
    return ss.str();
}

static void viewProfile(const string& studentId) {
    vector<vector<string>> students = readCsvRows(STUDENT_FILE);

    for (size_t i = 0; i < students.size(); ++i) {
        if (getCell(students[i], 0) == studentId) {
            cout << "\n===== VIEW PROFILE =====\n";
            cout << "ID        : " << getCell(students[i], 0) << "\n";
            cout << "Full Name : " << getCell(students[i], 1) << "\n";
            cout << "Email     : " << getCell(students[i], 2) << "\n";
            cout << "Phone     : " << getCell(students[i], 4) << "\n";
            cout << "Address   : " << getCell(students[i], 5) << "\n";
            cout << "Major     : " << getCell(students[i], 6) << "\n";
            cout << "Class     : " << getCell(students[i], 7) << "\n";
            return;
        }
    }

    cout << "Khong tim thay student_id: " << studentId << "\n";
}

static void viewScore(const string& studentId) {
    vector<vector<string>> enrollments = readCsvRows(ENROLL_FILE);
    vector<vector<string>> courses = readCourseFile();

    cout << "\n===== VIEW SCORE =====\n";

    const int w1 = 12, w2 = 28, w3 = 10, w4 = 12;
    cout << left << setw(w1) << "Course ID"
         << setw(w2) << "Course Name"
         << setw(w3) << "Score"
         << setw(w4) << "Result" << "\n";
    cout << string(w1 + w2 + w3 + w4, '-') << "\n";

    bool found = false;
    for (size_t i = 0; i < enrollments.size(); ++i) {
        if (getCell(enrollments[i], 1) != studentId) continue;

        string courseId = getCell(enrollments[i], 2);
        string courseName = findCourseName(courseId, courses);
        string score = getCell(enrollments[i], 5);
        string result = getCell(enrollments[i], 6);

        if (score.empty()) score = "N/A";
        if (result.empty()) result = "N/A";

        cout << left << setw(w1) << courseId
             << setw(w2) << courseName
             << setw(w3) << score
             << setw(w4) << result << "\n";
        found = true;
    }

    if (!found) {
        cout << "Khong co diem nao cho sinh vien nay.\n";
    }
}

static void viewCourses() {
    vector<vector<string>> courses = readCourseFile();

    cout << "\n===== VIEW COURSES =====\n";
    if (courses.empty()) {
        cout << "Khong co du lieu course.csv hoac file rong.\n";
        return;
    }

    const int w1 = 12, w2 = 28, w3 = 8, w4 = 14, w5 = 14, w6 = 14, w7 = 14, w8 = 14;
    cout << left << setw(w1) << "Course ID"
         << setw(w2) << "Course Name"
         << setw(w3) << "Credit"
         << setw(w4) << "Teacher ID"
         << setw(w5) << "Max Students"
         << setw(w6) << "Tuition Fee"
         << setw(w7) << "Start Date"
         << setw(w8) << "End Date" << "\n";
    cout << string(w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8, '-') << "\n";

    for (size_t i = 0; i < courses.size(); ++i) {
        cout << left << setw(w1) << getCell(courses[i], 0)
             << setw(w2) << getCell(courses[i], 1)
             << setw(w3) << getCell(courses[i], 2)
             << setw(w4) << getCell(courses[i], 3)
             << setw(w5) << getCell(courses[i], 4)
             << setw(w6) << getCell(courses[i], 5)
             << setw(w7) << getCell(courses[i], 6)
             << setw(w8) << getCell(courses[i], 7) << "\n";
    }
}

static void enrollCourse(const string& studentId) {
    cout << "Nhap course_id can dang ky: ";
    string courseId;
    getline(cin, courseId);
    courseId = trim(courseId);

    if (courseId.empty()) {
        cout << "course_id khong hop le.\n";
        return;
    }

    vector<vector<string>> courses = readCourseFile();
    vector<string> selectedCourse;
    if (!findCourseRow(courseId, courses, selectedCourse)) {
        cout << "course_id khong ton tai trong course.csv.\n";
        return;
    }

    vector<vector<string>> enrollments = readCsvRows(ENROLL_FILE);
    if (enrollmentExists(studentId, courseId, enrollments)) {
        cout << "Ban da dang ky course nay roi.\n";
        return;
    }

    string enrollmentId = nextEnrollmentId(enrollments);
    string enrollDate = todayDDMMYYYY();
    string tuitionFee = getCell(selectedCourse, 5);

    ofstream fout(ENROLL_FILE, ios::app);
    if (!fout.is_open()) {
        cout << "Khong mo duoc file: " << ENROLL_FILE << "\n";
        return;
    }

    fout << enrollmentId << " | "
         << studentId << " | "
         << courseId << " | "
         << enrollDate << " | "
         << "Registered | "
         << "N/A | "
         << "N/A | "
         << tuitionFee << " | "
         << "Unpaid"
         << "\n";

    cout << "Dang ky khoa hoc thanh cong!\n";
}

static void viewEnrollment(const string& studentId) {
    vector<vector<string>> enrollments = readCsvRows(ENROLL_FILE);
    vector<vector<string>> courses = readCourseFile();

    cout << "\n===== VIEW ENROLLMENT =====\n";

    const int w1 = 14, w2 = 12, w3 = 24, w4 = 14, w5 = 14, w6 = 14, w7 = 16;
    cout << left << setw(w1) << "Enroll ID"
         << setw(w2) << "Course ID"
         << setw(w3) << "Course Name"
         << setw(w4) << "Enroll Date"
         << setw(w5) << "Status"
         << setw(w6) << "Tuition Fee"
         << setw(w7) << "Payment Status" << "\n";
    cout << string(w1 + w2 + w3 + w4 + w5 + w6 + w7, '-') << "\n";

    bool found = false;
    for (size_t i = 0; i < enrollments.size(); ++i) {
        if (getCell(enrollments[i], 1) != studentId) continue;

        string courseId = getCell(enrollments[i], 2);
        string courseName = findCourseName(courseId, courses);

        cout << left << setw(w1) << getCell(enrollments[i], 0)
             << setw(w2) << courseId
             << setw(w3) << courseName
             << setw(w4) << getCell(enrollments[i], 3)
             << setw(w5) << getCell(enrollments[i], 4)
             << setw(w6) << getCell(enrollments[i], 7)
             << setw(w7) << getCell(enrollments[i], 8) << "\n";

        found = true;
    }

    if (!found) {
        cout << "Sinh vien chua co dang ky nao.\n";
    }
}

static void cancelEnrollment(const string& studentId) {
    vector<vector<string>> enrollments = readCsvRows(ENROLL_FILE);
    if (enrollments.empty()) {
        cout << "Khong co du lieu enrollment de huy.\n";
        return;
    }

    cout << "Nhap course_id can huy: ";
    string courseId;
    getline(cin, courseId);
    courseId = trim(courseId);

    if (courseId.empty()) {
        cout << "course_id khong hop le.\n";
        return;
    }

    vector<vector<string>> newRows;
    bool removed = false;

    for (size_t i = 0; i < enrollments.size(); ++i) {
        bool isTarget = (getCell(enrollments[i], 1) == studentId &&
                         getCell(enrollments[i], 2) == courseId);
        if (isTarget) {
            removed = true;
            continue;
        }
        newRows.push_back(enrollments[i]);
    }

    if (!removed) {
        cout << "Khong tim thay dang ky phu hop de huy.\n";
        return;
    }

    if (writeCsvRows(ENROLL_FILE, newRows)) {
        cout << "Huy dang ky thanh cong!\n";
    }
}

}  // namespace

void studentCourseMenu(const string& studentId) {
    while (true) {
        cout << "\n========== Course Menu ==========\n";
        cout << "1. View profile - id\n";
        cout << "2. View score\n";
        cout << "3. View Courses\n";
        cout << "4. Enroll Course\n";
        cout << "5. View Enrollment\n";
        cout << "6. Cancel Enrollment\n";
        cout << "7. Logout\n";
        cout << "------------------------------------\n";

        int choice = readChoice();
        switch (choice) {
            case 1:
                viewProfile(studentId);
                break;
            case 2:
                viewScore(studentId);
                break;
            case 3:
                viewCourses();
                break;
            case 4:
                enrollCourse(studentId);
                break;
            case 5:
                viewEnrollment(studentId);
                break;
            case 6:
                cancelEnrollment(studentId);
                break;
            case 7:
                cout << "Dang xuat...\n";
                return;
            default:
                cout << "Lua chon khong hop le. Vui long nhap 1-7.\n";
                break;
        }
    }
}
