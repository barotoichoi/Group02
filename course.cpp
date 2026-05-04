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
        cout << "Cannot open file: " << filePath << "\n";
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
        cout << "Cannot write file: " << filePath << "\n";
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

static bool writePipeRows(const string& filePath, const vector<vector<string>>& rows) {
    ofstream fout(filePath);
    if (!fout.is_open()) {
        cout << "Cannot write file: " << filePath << "\n";
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

static int toIntSafe(const string& value) {
    try {
        return stoi(trim(value));
    } catch (...) {
        return 0;
    }
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

static bool parseDate(const string& value, tm& outDate) {
    if (value.size() != 10) return false;
    if ((value[2] != '/' && value[2] != '-') || (value[5] != '/' && value[5] != '-')) return false;

    try {
        const int day = stoi(value.substr(0, 2));
        const int month = stoi(value.substr(3, 2));
        const int year = stoi(value.substr(6, 4));

        outDate = {};
        outDate.tm_mday = day;
        outDate.tm_mon = month - 1;
        outDate.tm_year = year - 1900;
        outDate.tm_hour = 12;
        return day > 0 && month > 0 && month <= 12 && year > 1900;
    } catch (...) {
        return false;
    }
}

static string addDaysToDate(const string& value, int days) {
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
#ifdef _WIN32
#ifdef _MSC_VER
    localtime_s(&result, &timestamp);
#else
    result = *localtime(&timestamp);
#endif
#else
    localtime_r(&timestamp, &result);
#endif

    char buffer[16];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y", &result);
    return string(buffer);
}

static bool isUnpaidStatus(const string& status) {
    const string normalized = trim(status);
    return normalized.empty() ||
           normalized == "Unpaid" ||
           normalized == "unpaid" ||
           normalized == "Unpaid";
}

static int readChoice() {
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

static int countCourseEnrollments(const vector<vector<string>>& enrollments, const string& courseId) {
    int count = 0;

    for (size_t i = 0; i < enrollments.size(); ++i) {
        if (getCell(enrollments[i], 2) == courseId) {
            ++count;
        }
    }

    return count;
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

static bool isGradeAvailable(const string& grade) {
    const string normalized = trim(grade);
    return !normalized.empty() &&
           normalized != "N/A" &&
           normalized != "n/a";
}

static void appendEnrollment(const string& studentId, const vector<string>& selectedCourse) {
    vector<vector<string>> enrollments = readCsvRows(ENROLL_FILE);
    const string courseId = getCell(selectedCourse, 0);

    if (enrollmentExists(studentId, courseId, enrollments)) {
        cout << "You have already enrolled in this course.\n";
        return;
    }

    const string enrollmentId = nextEnrollmentId(enrollments);
    const string enrollDate = todayDDMMYYYY();
    const string tuitionFee = getCell(selectedCourse, 5);

    ofstream fout(ENROLL_FILE, ios::app);
    if (!fout.is_open()) {
        cout << "Cannot open file: " << ENROLL_FILE << "\n";
        return;
    }

    fout << enrollmentId << " | "
         << studentId << " | "
         << courseId << " | "
         << enrollDate << " | "
         << "Registered | "
         << " | "
         << " | "
         << tuitionFee << " | "
         << "Unpaid"
         << "\n";

    cout << "Course enrollment successful!\n";
}

static void ensureStudentProfileFields(vector<string>& student) {
    if (student.size() == 9 && getCell(student, 8) == "student") {
        student.insert(student.begin() + 8, "");
    }

    while (student.size() < 9) {
        student.push_back("");
    }
}

static bool hasMissingProfileInfo(const vector<string>& student) {
    for (size_t i = 4; i <= 8; ++i) {
        if (trim(getCell(student, i)).empty()) {
            return true;
        }
    }

    return false;
}

static string displayProfileValue(const vector<string>& student, size_t index) {
    const string value = getCell(student, index);
    return trim(value).empty() ? "[Not updated]" : value;
}

static string profileDisplayText(const string& value) {
    return trim(value).empty() ? "[Not updated]" : value;
}

static string readProfileUpdateValue(const string& label, const string& currentValue) {
    cout << label << " current: " << profileDisplayText(currentValue) << "\n";
    cout << "New " << label << " (press Enter to keep current): ";

    string value;
    getline(cin, value);
    value = trim(value);

    return value.empty() ? currentValue : value;
}

static void updateProfile(const string& studentId) {
    vector<vector<string>> students = readCsvRows(STUDENT_FILE);
    const vector<pair<size_t, string>> fields = {
        {1, "Full name"},
        {2, "Email"},
        {4, "Date of birth"},
        {5, "Phone"},
        {6, "Address"},
        {7, "Major"},
        {8, "Class"}
    };

    for (size_t i = 0; i < students.size(); ++i) {
        if (getCell(students[i], 0) != studentId) {
            continue;
        }

        ensureStudentProfileFields(students[i]);

        cout << "\n--- UPDATE INFORMATION ---\n";
        for (size_t j = 0; j < fields.size(); ++j) {
            const size_t fieldIndex = fields[j].first;
            students[i][fieldIndex] = readProfileUpdateValue(fields[j].second, getCell(students[i], fieldIndex));
        }

        if (writePipeRows(STUDENT_FILE, students)) {
            cout << "=> Success!\n";
        }
        return;
    }

    cout << "Student information not found.\n";
}

static void viewProfile(const string& studentId);

static void personalInformationMenu(const string& studentId) {
    while (true) {
        cout << "\n===== PERSONAL INFORMATION =====\n";
        cout << "1. View profile\n";
        cout << "2. Update profile\n";
        cout << "0. Back\n";

        switch (readChoice()) {
            case 1:
                viewProfile(studentId);
                break;
            case 2:
                updateProfile(studentId);
                break;
            case 0:
                return;
            default:
                cout << "Invalid choice.\n";
                break;
        }
    }
}

static void viewProfile(const string& studentId) {
    vector<vector<string>> students = readCsvRows(STUDENT_FILE);

    for (size_t i = 0; i < students.size(); ++i) {
        if (getCell(students[i], 0) == studentId) {
            ensureStudentProfileFields(students[i]);

            cout << "\n===== VIEW PROFILE =====\n";
            cout << "ID        : " << getCell(students[i], 0) << "\n";
            cout << "Full Name : " << getCell(students[i], 1) << "\n";
            cout << "Email     : " << getCell(students[i], 2) << "\n";
            cout << "Date of birth: " << displayProfileValue(students[i], 4) << "\n";
            cout << "Phone     : " << displayProfileValue(students[i], 5) << "\n";
            cout << "Address   : " << displayProfileValue(students[i], 6) << "\n";
            cout << "Major     : " << displayProfileValue(students[i], 7) << "\n";
            cout << "Class     : " << displayProfileValue(students[i], 8) << "\n";

            if (hasMissingProfileInfo(students[i])) {
                cout << "Some profile information has not been updated yet.\n";
            }
            return;
        }
    }

    cout << "student_id not found: " << studentId << "\n";
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

        cout << left << setw(w1) << courseId
             << setw(w2) << courseName
             << setw(w3) << score
             << setw(w4) << result << "\n";
        found = true;
    }

    if (!found) {
        cout << "No grades found for this student.\n";
    }
}

static bool isFinishedCourse(const vector<string>& enrollment) {
    const string score = trim(getCell(enrollment, 5));
    const string result = trim(getCell(enrollment, 6));

    return isGradeAvailable(score) ||
           (!result.empty() && result != "N/A" && result != "Result");
}

static void printRegisteredCourseTable(const vector<vector<string>>& enrollments,
                                       const vector<vector<string>>& courses,
                                       const string& studentId,
                                       bool finished) {
    const int w1 = 12, w2 = 28, w3 = 10, w4 = 14, w5 = 14, w6 = 10, w7 = 12, w8 = 14;
    bool found = false;

    cout << left << setw(w1) << "Course ID"
         << setw(w2) << "Course Name"
         << setw(w3) << "Credits"
         << setw(w4) << "Start Date"
         << setw(w5) << "End Date"
         << setw(w6) << "Score"
         << setw(w7) << "Result"
         << setw(w8) << "Status" << "\n";
    cout << string(w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8, '-') << "\n";

    for (size_t i = 0; i < enrollments.size(); ++i) {
        if (getCell(enrollments[i], 1) != studentId) continue;
        if (isFinishedCourse(enrollments[i]) != finished) continue;

        vector<string> course;
        const string courseId = getCell(enrollments[i], 2);
        findCourseRow(courseId, courses, course);

        string score = getCell(enrollments[i], 5);
        string result = getCell(enrollments[i], 6);
        cout << left << setw(w1) << courseId
             << setw(w2) << findCourseName(courseId, courses)
             << setw(w3) << getCell(course, 2)
             << setw(w4) << getCell(course, 6)
             << setw(w5) << getCell(course, 7)
             << setw(w6) << score
             << setw(w7) << result
             << setw(w8) << (finished ? "Completed" : "In progress")
             << "\n";
        found = true;
    }

    if (!found) {
        cout << (finished ? "No completed courses.\n" : "No in-progress courses.\n");
    }
}

static void viewRegisteredCourses(const string& studentId) {
    vector<vector<string>> enrollments = readCsvRows(ENROLL_FILE);
    vector<vector<string>> courses = readCourseFile();
    cout << "\n===== VIEW COURSES =====\n";

    bool hasEnrollment = false;
    for (size_t i = 0; i < enrollments.size(); ++i) {
        if (getCell(enrollments[i], 1) == studentId) {
            hasEnrollment = true;
            break;
        }
    }

    if (!hasEnrollment) {
        cout << "The student has not enrolled in any courses.\n";
        return;
    }

    cout << "\n--- In-progress courses ---\n";
    printRegisteredCourseTable(enrollments, courses, studentId, false);

    cout << "\n--- Completed courses ---\n";
    printRegisteredCourseTable(enrollments, courses, studentId, true);
}

static void enrollCourse(const string& studentId) {
    vector<vector<string>> courses = readCourseFile();
    vector<vector<string>> enrollments = readCsvRows(ENROLL_FILE);
    vector<vector<string>> availableCourses;

    cout << "\n===== COURSE ENROLLMENT =====\n";

    const int wNo = 6, wId = 12, wName = 28, wCredits = 10, wStart = 14,
              wEnd = 14, wSeats = 12, wFee = 14;
    cout << left << setw(wNo) << "No."
         << setw(wId) << "Course ID"
         << setw(wName) << "Course Name"
         << setw(wCredits) << "Credits"
         << setw(wStart) << "Start Date"
         << setw(wEnd) << "End Date"
         << setw(wSeats) << "Seats"
         << setw(wFee) << "Fee/Credit" << "\n";
    cout << string(wNo + wId + wName + wCredits + wStart + wEnd + wSeats + wFee, '-') << "\n";

    for (size_t i = 0; i < courses.size(); ++i) {
        const string courseId = getCell(courses[i], 0);
        const int maxStudents = toIntSafe(getCell(courses[i], 4));
        const int registeredCount = countCourseEnrollments(enrollments, courseId);

        if (courseId.empty() ||
            enrollmentExists(studentId, courseId, enrollments) ||
            maxStudents <= 0 ||
            registeredCount >= maxStudents) {
            continue;
        }

        availableCourses.push_back(courses[i]);
        const string seatDisplay = to_string(maxStudents - registeredCount) + "/" + to_string(maxStudents);

        cout << left << setw(wNo) << availableCourses.size()
             << setw(wId) << courseId
             << setw(wName) << getCell(courses[i], 1)
             << setw(wCredits) << getCell(courses[i], 2)
             << setw(wStart) << getCell(courses[i], 6)
             << setw(wEnd) << getCell(courses[i], 7)
             << setw(wSeats) << seatDisplay
             << setw(wFee) << getCell(courses[i], 5) << "\n";
    }

    if (availableCourses.empty()) {
        cout << "No courses are currently available for enrollment.\n";
        return;
    }

    cout << "0. Back\n";
    const int choice = readChoice();
    if (choice == 0) {
        return;
    }
    if (choice < 1 || static_cast<size_t>(choice) > availableCourses.size()) {
        cout << "Invalid course number.\n";
        return;
    }

    appendEnrollment(studentId, availableCourses[choice - 1]);
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
        cout << "The student has no enrollments.\n";
    }
}

static void viewCourseGradeDetail(const string& studentId, const string& selectedCourseId) {
    vector<vector<string>> enrollments = readCsvRows(ENROLL_FILE);
    vector<vector<string>> courses = readCourseFile();

    cout << "\n===== COURSE GRADE DETAIL =====\n";

    for (size_t i = 0; i < enrollments.size(); ++i) {
        if (getCell(enrollments[i], 1) == studentId &&
            getCell(enrollments[i], 2) == selectedCourseId) {
            cout << "Course: " << findCourseName(selectedCourseId, courses)
                 << " (" << selectedCourseId << ")\n";
            cout << "Enrollment date: " << getCell(enrollments[i], 3) << '\n';
            cout << "Enrollment status: " << getCell(enrollments[i], 4) << '\n';
            cout << "Score: " << getCell(enrollments[i], 5) << '\n';
            cout << "Result: " << getCell(enrollments[i], 6) << '\n';
            return;
        }
    }

    cout << "Course grade information not found.\n";
}

static void viewTuitionDetail(const string& studentId, const string& selectedCourseId) {
    vector<vector<string>> enrollments = readCsvRows(ENROLL_FILE);
    vector<vector<string>> courses = readCourseFile();

    cout << "\n===== TUITION DETAIL =====\n";

    for (size_t i = 0; i < enrollments.size(); ++i) {
        if (getCell(enrollments[i], 1) == studentId &&
            getCell(enrollments[i], 2) == selectedCourseId) {
            vector<string> course;
            string deadline = "Unknown";
            if (findCourseRow(selectedCourseId, courses, course)) {
                deadline = addDaysToDate(getCell(course, 7), -7);
            }

            cout << "Course: " << findCourseName(selectedCourseId, courses)
                 << " (" << selectedCourseId << ")\n";
            cout << "Tuition: " << getCell(enrollments[i], 7) << '\n';
            cout << "Payment status: " << getCell(enrollments[i], 8) << '\n';
            cout << "Payment deadline: " << deadline << '\n';
            return;
        }
    }

    cout << "Tuition information not found.\n";
}

static bool updatePaymentStatus(const string& studentId, const string& selectedCourseId, const string& status) {
    vector<vector<string>> enrollments = readCsvRows(ENROLL_FILE);

    for (size_t i = 0; i < enrollments.size(); ++i) {
        if (getCell(enrollments[i], 1) == studentId &&
            getCell(enrollments[i], 2) == selectedCourseId) {
            while (enrollments[i].size() < 9) {
                enrollments[i].push_back("");
            }

            enrollments[i][8] = status;
            return writePipeRows(ENROLL_FILE, enrollments);
        }
    }

    cout << "Tuition information not found.\n";
    return false;
}

static void viewTuitionList(const string& studentId) {
    vector<vector<string>> enrollments = readCsvRows(ENROLL_FILE);
    vector<vector<string>> courses = readCourseFile();

    cout << "\n===== TUITION =====\n";

    const int w1 = 12, w2 = 28, w3 = 14, w4 = 16, w5 = 14;
    cout << left << setw(w1) << "Course ID"
         << setw(w2) << "Course Name"
         << setw(w3) << "Tuition"
         << setw(w4) << "Payment Status"
         << setw(w5) << "Deadline" << "\n";
    cout << string(w1 + w2 + w3 + w4 + w5, '-') << "\n";

    bool found = false;
    for (size_t i = 0; i < enrollments.size(); ++i) {
        if (getCell(enrollments[i], 1) != studentId) continue;

        const string courseId = getCell(enrollments[i], 2);
        vector<string> course;
        string deadline = "Unknown";
        if (findCourseRow(courseId, courses, course)) {
            deadline = addDaysToDate(getCell(course, 7), -7);
        }

        cout << left << setw(w1) << courseId
             << setw(w2) << findCourseName(courseId, courses)
             << setw(w3) << getCell(enrollments[i], 7)
             << setw(w4) << getCell(enrollments[i], 8)
             << setw(w5) << deadline << "\n";
        found = true;
    }

    if (!found) {
        cout << "The student has no tuition records.\n";
    }
}

static void changePassword(const string& studentId) {
    vector<vector<string>> students = readCsvRows(STUDENT_FILE);

    for (size_t i = 0; i < students.size(); ++i) {
        if (getCell(students[i], 0) != studentId) continue;

        string currentPassword;
        string newPassword;
        string confirmPassword;

        cout << "Current password: ";
        getline(cin, currentPassword);
        if (getCell(students[i], 3) != currentPassword) {
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

        students[i][3] = newPassword;
        if (writePipeRows(STUDENT_FILE, students)) {
            cout << "Password changed successfully.\n";
        }
        return;
    }

    cout << "Student information not found.\n";
}

static void cancelEnrollment(const string& studentId) {
    vector<vector<string>> enrollments = readCsvRows(ENROLL_FILE);
    if (enrollments.empty()) {
        cout << "No enrollment data to cancel.\n";
        return;
    }

    cout << "Enter course_id to cancel: ";
    string courseId;
    getline(cin, courseId);
    courseId = trim(courseId);

    if (courseId.empty()) {
        cout << "Invalid course_id.\n";
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
        cout << "No matching enrollment found to cancel.\n";
        return;
    }

    if (writeCsvRows(ENROLL_FILE, newRows)) {
        cout << "Enrollment cancelled successfully!\n";
    }
}

static void registrationMenu(const string& studentId) {
    while (true) {
        cout << "\n===== COURSE REGISTRATION =====\n";
        cout << "1. View enrollment\n";
        cout << "2. Enroll course\n";
        cout << "3. Cancel enrollment\n";
        cout << "0. Back\n";

        const int choice = readChoice();
        switch (choice) {
            case 1:
                viewEnrollment(studentId);
                break;
            case 2:
                enrollCourse(studentId);
                break;
            case 3:
                cancelEnrollment(studentId);
                break;
            case 0:
                return;
            default:
                cout << "Invalid choice.\n";
                break;
        }
    }
}

struct StudentNotification {
    int type;
    string courseId;
    string courseName;
    string tuitionFee;
    string paymentDeadline;
    vector<string> course;
};

static void payTuitionFromNotification(const string& studentId, const StudentNotification& notification) {
    cout << "\n===== TUITION PAYMENT =====\n";
    cout << "Course: " << notification.courseName << " (" << notification.courseId << ")\n";
    cout << "Tuition: " << notification.tuitionFee << '\n';
    cout << "Payment deadline: " << notification.paymentDeadline << '\n';
    cout << "1. Confirm payment\n";
    cout << "2. Cancel payment\n";
    cout << "0. Back\n";

    const int choice = readChoice();
    switch (choice) {
        case 1:
            if (updatePaymentStatus(studentId, notification.courseId, "Paid")) {
                cout << "Payment completed successfully.\n";
            }
            break;
        case 2:
            cout << "Payment cancelled.\n";
            break;
        case 0:
            return;
        default:
            cout << "Invalid choice.\n";
            break;
    }
}

static vector<StudentNotification> buildNotifications(const string& studentId) {
    vector<vector<string>> courses = readCourseFile();
    vector<vector<string>> enrollments = readCsvRows(ENROLL_FILE);
    vector<StudentNotification> notifications;

    for (size_t i = 0; i < courses.size(); ++i) {
        const string courseId = getCell(courses[i], 0);
        const int maxStudents = toIntSafe(getCell(courses[i], 4));
        const int registeredCount = countCourseEnrollments(enrollments, courseId);

        if (maxStudents > 0 &&
            registeredCount < maxStudents &&
            !enrollmentExists(studentId, courseId, enrollments)) {
            notifications.push_back({1, courseId, getCell(courses[i], 1), "", "", courses[i]});
        }
    }

    for (size_t i = 0; i < enrollments.size(); ++i) {
        if (getCell(enrollments[i], 1) != studentId) continue;

        const string courseId = getCell(enrollments[i], 2);
        const string courseName = findCourseName(courseId, courses);

        if (isGradeAvailable(getCell(enrollments[i], 5))) {
            notifications.push_back({2, courseId, courseName, "", "", {}});
        }

        if (isUnpaidStatus(getCell(enrollments[i], 8))) {
            vector<string> course;
            string deadline = "Unknown";
            if (findCourseRow(courseId, courses, course)) {
                deadline = addDaysToDate(getCell(course, 7), -7);
            }

            const string tuitionFee = getCell(enrollments[i], 7).empty() ? "0" : getCell(enrollments[i], 7);
            notifications.push_back({3, courseId, courseName, tuitionFee, deadline, {}});
        }
    }

    return notifications;
}

static void showNotifications(const string& studentId) {
    const vector<StudentNotification> notifications = buildNotifications(studentId);

    cout << "\n===== NOTIFICATIONS =====\n";

    if (notifications.empty()) {
        cout << "There are no new notifications.\n";
        return;
    }

    for (size_t i = 0; i < notifications.size(); ++i) {
        cout << (i + 1) << ". ";
        if (notifications[i].type == 1) {
            cout << "New open course: \"" << notifications[i].courseName
                 << "\" is available for enrollment.\n";
        } else if (notifications[i].type == 2) {
            cout << "Grade notification: \"" << notifications[i].courseName
                 << "\" has a grade update.\n";
        } else {
            cout << "Unpaid tuition: \"" << notifications[i].courseName
                 << "\". Tuition: " << notifications[i].tuitionFee
                 << ". Payment deadline: " << notifications[i].paymentDeadline << ".\n";
        }
    }

    cout << "0. Back\n";
    const int choice = readChoice();

    if (choice <= 0 || static_cast<size_t>(choice) > notifications.size()) {
        return;
    }

    const StudentNotification& notification = notifications[choice - 1];

    if (notification.type == 1) {
        cout << "\n===== COURSE ENROLLMENT =====\n";
        cout << "Course: " << notification.courseName << " (" << notification.courseId << ")\n";
        cout << "1. Enroll in this course\n";
        cout << "0. Back\n";

        if (readChoice() == 1) {
            appendEnrollment(studentId, notification.course);
        }
    } else if (notification.type == 2) {
        viewCourseGradeDetail(studentId, notification.courseId);
    } else {
        payTuitionFromNotification(studentId, notification);
    }
}

}  // namespace

void studentCourseMenu(const string& studentId) {
    while (true) {
        cout << "\n========== Course Menu ==========\n";
        cout << "1. Personal information\n";
        cout << "2. View courses\n";
        cout << "3. Course registration\n";
        cout << "4. Notifications\n";
        cout << "5. View tuition\n";
        cout << "6. Change password\n";
        cout << "0. Logout\n";
        cout << "------------------------------------\n";

        int choice = readChoice();
        switch (choice) {
            case 0:
                cout << "Logging out...\n";
                return;
            case 1:
                personalInformationMenu(studentId);
                break;
            case 2:
                viewRegisteredCourses(studentId);
                break;
            case 3:
                registrationMenu(studentId);
                break;
            case 4:
                showNotifications(studentId);
                break;
            case 5:
                viewTuitionList(studentId);
                break;
            case 6:
                changePassword(studentId);
                break;
            default:
                cout << "Invalid choice. Please choose 0-6.\n";
                break;
        }
    }
}
