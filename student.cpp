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
    cout << "Enter your choice: ";

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
            cout << "\n===== STUDENT INFORMATION =====\n";
            cout << "ID: " << getField(student.fields, 0) << '\n';
            cout << "Name: " << getField(student.fields, 1) << '\n';
            cout << "Email: " << getField(student.fields, 2) << '\n';
            cout << "Date of birth: " << getField(student.fields, studentInfoIndex(4, student.fields)) << '\n';
            cout << "Phone: " << getField(student.fields, studentInfoIndex(5, student.fields)) << '\n';
            cout << "Address: " << getField(student.fields, studentInfoIndex(6, student.fields)) << '\n';
            cout << "Major: " << getField(student.fields, studentInfoIndex(7, student.fields)) << '\n';
            cout << "Class: " << getField(student.fields, studentInfoIndex(8, student.fields)) << '\n';
            return;
        }
    }

    cout << "Student information not found.\n";
}

void printCourseList(const string& baseDir) {
    ifstream file(joinPath(baseDir, "course.csv"));
    if (!file.is_open()) {
        cout << "Cannot open file course.csv!\n";
        return;
    }

    cout << "\n===== COURSE LIST =====\n";
    string line;
    while (getline(file, line)) {
        if (trim(line).empty()) continue;

        vector<string> fields = split(line);
        for (string& field : fields) {
            field = trim(field);
        }

        cout << "ID: " << getField(fields, 0)
             << " | Name: " << getField(fields, 1)
             << " | Credits: " << getField(fields, 2)
             << " | Tuition: " << getField(fields, 5)
             << '\n';
    }
}

void printEnrollmentList(const string& baseDir, const string& studentId) {
    ifstream file(joinPath(baseDir, "enrollment.csv"));
    if (!file.is_open()) {
        cout << "Cannot open file enrollment.csv!\n";
        return;
    }

    cout << "\n===== ENROLLED COURSES =====\n";
    bool found = false;
    string line;
    while (getline(file, line)) {
        if (trim(line).empty()) continue;

        vector<string> fields = split(line);
        for (string& field : fields) {
            field = trim(field);
        }

        if (getField(fields, 1) == studentId) {
            cout << "Enrollment ID: " << getField(fields, 0)
                 << " | Course ID: " << getField(fields, 2)
                 << " | Enrollment date: " << getField(fields, 3)
                 << " | Tuition: " << getField(fields, 4)
                 << " | Grade: " << getField(fields, 5)
                 << '\n';
            found = true;
        }
    }

    if (!found) {
        cout << "The student has not enrolled in any courses.\n";
    }
}

vector<vector<string>> loadPipeRows(const string& filePath) {
    vector<vector<string>> rows;
    ifstream file(filePath);

    if (!file.is_open()) {
        cout << "Cannot open file " << filePath << "!\n";
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

vector<string> findCourseById(const vector<vector<string>>& courses, const string& courseId) {
    for (const vector<string>& course : courses) {
        if (getField(course, 0) == courseId) {
            return course;
        }
    }

    return {};
}

bool parseDate(const string& value, tm& outDate) {
    if (value.size() != 10) return false;
    if ((value[2] != '/' && value[2] != '-') || (value[5] != '/' && value[5] != '-')) return false;

    try {
        outDate = {};
        outDate.tm_mday = stoi(value.substr(0, 2));
        outDate.tm_mon = stoi(value.substr(3, 2)) - 1;
        outDate.tm_year = stoi(value.substr(6, 4)) - 1900;
        outDate.tm_hour = 12;
        return outDate.tm_mday > 0 && outDate.tm_mon >= 0 && outDate.tm_mon < 12;
    } catch (...) {
        return false;
    }
}

string addDaysToDate(const string& value, int days) {
    tm date{};
    if (!parseDate(value, date)) {
        return "Unknown";
    }

    time_t timestamp = mktime(&date);
    if (timestamp == static_cast<time_t>(-1)) {
        return "Unknown";
    }

    timestamp += static_cast<time_t>(days) * 24 * 60 * 60;
    tm result{};
#ifdef _MSC_VER
    localtime_s(&result, &timestamp);
#else
    result = *localtime(&timestamp);
#endif

    char buffer[16];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y", &result);
    return string(buffer);
}

bool isUnpaidStatus(const string& status) {
    const string normalized = trim(status);
    return normalized.empty() ||
           normalized == "Unpaid" ||
           normalized == "unpaid" ||
           normalized == "Unpaid";
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
        cout << "Cannot write file " << filePath << "!\n";
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

    cout << "\n===== COURSE DETAILS =====\n";
    cout << "Course ID: " << getField(course, 0) << '\n';
    cout << "Course name: " << getField(course, 1) << '\n';
    cout << "Credits: " << getField(course, 2) << '\n';
    cout << "Teacher ID: " << getField(course, 3) << '\n';
    cout << "Available seats: " << (maxStudents - registeredCount) << "/" << maxStudents << '\n';
    cout << "Fee per credit: " << getField(course, 5) << '\n';
    cout << "Start date: " << getField(course, 6) << '\n';
    cout << "End date: " << getField(course, 7) << '\n';
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
        cout << "You have already enrolled in this course.\n";
        return;
    }

    if (maxStudents <= 0 || registeredCount >= maxStudents) {
        cout << "The course is closed or has no available seats.\n";
        return;
    }

    cout << "\n1. Enroll in this course\n";
    cout << "0. Back\n";

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
        cout << "Course enrollment successful.\n";
    }
}

void showGradeDetail(const string& baseDir, const string& studentId, const string& courseId) {
    const vector<vector<string>> courses = loadPipeRows(joinPath(baseDir, "course.csv"));
    const vector<vector<string>> enrollments = loadPipeRows(joinPath(baseDir, "enrollment.csv"));

    cout << "\n===== COURSE GRADE INFORMATION =====\n";
    for (const vector<string>& enrollment : enrollments) {
        if (getField(enrollment, 1) == studentId && getField(enrollment, 2) == courseId) {
            cout << "Course: " << findCourseName(courses, courseId) << " (" << courseId << ")\n";
            cout << "Enrollment date: " << getField(enrollment, 3) << '\n';
            cout << "Enrollment status: " << getField(enrollment, 4) << '\n';
            cout << "Grade: " << getField(enrollment, 5) << '\n';
            return;
        }
    }

    cout << "Course information not found.\n";
}

struct StudentNotification {
    int type;
    string courseId;
    string courseName;
    string tuitionFee;
    string paymentDeadline;
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
            notifications.push_back({1, courseId, getField(course, 1), "", "", course});
        }
    }

    for (const vector<string>& enrollment : enrollments) {
        if (getField(enrollment, 1) != studentId) {
            continue;
        }

        const string grade = trim(getField(enrollment, 5));
        if (!grade.empty() && grade != "0") {
            const string courseId = getField(enrollment, 2);
            notifications.push_back({2, courseId, findCourseName(courses, courseId), "", "", {}});
        }

        if (isUnpaidStatus(getField(enrollment, 8))) {
            const string courseId = getField(enrollment, 2);
            const vector<string> course = findCourseById(courses, courseId);
            const string deadline = course.empty() ? "Unknown" : addDaysToDate(getField(course, 7), -7);
            const string tuitionFee = getField(enrollment, 7).empty() ? "0" : getField(enrollment, 7);
            notifications.push_back({3, courseId, findCourseName(courses, courseId), tuitionFee, deadline, {}});
        }
    }

    return notifications;
}

void showNotifications(const string& baseDir, const string& studentId) {
    const vector<StudentNotification> notifications = buildNotifications(baseDir, studentId);

    cout << "\n===== NOTIFICATIONS =====\n";
    if (notifications.empty()) {
        cout << "There are no new notifications.\n";
        return;
    }

    for (size_t i = 0; i < notifications.size(); ++i) {
        cout << (i + 1) << ". ";
        if (notifications[i].type == 1) {
            cout << "You have course \"" << notifications[i].courseName
                 << "\" available for enrollment (school notification)\n";
        } else if (notifications[i].type == 2) {
            cout << "You have a grade notification for course \"" << notifications[i].courseName
                 << "\" (notification from teacher)\n";
        } else {
            cout << "You have not paid tuition for course \"" << notifications[i].courseName
                 << "\". Tuition: " << notifications[i].tuitionFee
                 << ". Payment deadline: " << notifications[i].paymentDeadline << '\n';
        }
    }
    cout << "0. Back\n";

    const int choice = readChoice();
    if (choice <= 0 || static_cast<size_t>(choice) > notifications.size()) {
        return;
    }

    const StudentNotification& notification = notifications[choice - 1];
    if (notification.type == 1) {
        registerCourseFromNotification(baseDir, studentId, notification.course);
    } else if (notification.type == 2) {
        showGradeDetail(baseDir, studentId, notification.courseId);
    } else {
        cout << "\n===== TUITION DETAILS =====\n";
        cout << "Course: " << notification.courseName << " (" << notification.courseId << ")\n";
        cout << "Tuition: " << notification.tuitionFee << '\n';
        cout << "Payment deadline: " << notification.paymentDeadline << '\n';
    }
}

bool saveStudentList(const string& filePath, const vector<UserStudentRecord>& students) {
    ofstream file(filePath);
    if (!file.is_open()) {
        cout << "Cannot write file " << filePath << "!\n";
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

            cout << "Current password: ";
            getline(cin, oldPassword);
            if (getField(student.fields, 3) != oldPassword) {
                cout << "Current password is incorrect.\n";
                return;
            }

            cout << "New password: ";
            getline(cin, newPassword);
            newPassword = trim(newPassword);
            if (newPassword.empty()) {
                cout << "New password cannot be empty.\n";
                return;
            }

            cout << "Confirm new password: ";
            getline(cin, confirmPassword);
            if (newPassword != trim(confirmPassword)) {
                cout << "Password confirmation does not match.\n";
                return;
            }

            student.fields[3] = newPassword;
            if (saveStudentList(filePath, students)) {
                cout << "Password changed successfully.\n";
            }
            return;
        }
    }

    cout << "Student information not found.\n";
}
}


vector<UserStudentRecord> loadUserList(const string& filePath) {
    vector<UserStudentRecord> users;
    ifstream file(filePath);
    string line;

    if (!file.is_open()) {
        cout << "Cannot open file!\n";
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
        cout << "\n========== STUDENT MENU ==========\n";
        cout << "1. View personal information\n";
        cout << "2. View course list\n";
        cout << "3. View enrolled courses\n";
        cout << "4. Change password\n";
        cout << "5. Notifications\n";
        cout << "0. Log out\n";

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
                cout << "Log out...\n";
                return;
            default:
                cout << "Invalid choice.\n";
                break;
        }
    }
}

void runStudentLogin(const string& baseDir) {
    const string filePath = joinPath(baseDir, "student.csv");
    vector<UserStudentRecord> users = loadUserList(filePath);

    if (users.empty()) {
        cout << "No student records found or cannot open file: " << filePath << "\n";
        return;
    }

    string email, password;
    cout << "===== STUDENT LOGIN =====\n";
    cout << "Email: ";
    getline(cin, email);
    cout << "Password: ";
    getline(cin, password);

    for (const UserStudentRecord& u : users) {
        if (u.fields.size() >= 9 &&
            getField(u.fields, 2) == trim(email) &&
            getField(u.fields, 3) == trim(password) &&
            getField(u.fields, 8) == "student") {

            cout << "\nLogin successful!\n";
            // call course menu implemented in course.cpp
            studentCourseMenu(getField(u.fields, 0));
            return;
        }
    }

    cout << "\nIncorrect email or password!\n";
}

