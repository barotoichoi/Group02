#include "admin.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <cstdlib>

namespace {
int toIntSafe(const std::string& value);

std::string trim(const std::string& value) {
    const size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }

    const size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::vector<std::string> splitByPipe(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string token;

    while (std::getline(ss, token, '|')) {
        fields.push_back(trim(token));
    }

    return fields;
}

std::string joinPath(const std::string& dir, const std::string& fileName) {
    if (dir.empty() || dir == ".") {
        return fileName;
    }

    const char last = dir.back();
    if (last == '/' || last == '\\') {
        return dir + fileName;
    }

    return dir + "/" + fileName;
}

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int readChoice() {
    int choice;
    std::cout << "Enter your choice: ";

    if (!(std::cin >> choice)) {
        clearInput();
        return -1;
    }

    clearInput();
    return choice;
}

void pauseScreen() {
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
}

void exitProgram() {
    std::cout << "Program exited.\n";
    std::exit(0);
}

std::string getField(const UserAdminRecord& record, size_t index) {
    if (index >= record.fields.size()) {
        return "";
    }

    return record.fields[index];
}

std::string findTeacherName(const std::vector<UserAdminRecord>& teachers, const std::string& teacherId) {
    for (const UserAdminRecord& teacher : teachers) {
        if (getField(teacher, 0) == teacherId) {
            return getField(teacher, 1);
        }
    }

    return "Teacher not found";
}

std::string findStudentName(const std::vector<UserAdminRecord>& students, const std::string& studentId) {
    for (const UserAdminRecord& student : students) {
        if (getField(student, 0) == studentId) {
            return getField(student, 1);
        }
    }

    return "Student not found";
}

std::string findStudentClass(const std::vector<UserAdminRecord>& students, const std::string& studentId) {
    for (const UserAdminRecord& student : students) {
        if (getField(student, 0) == studentId) {
            const size_t classIndex = (student.fields.size() >= 10 && student.fields.back() == "student") ? 8 : 7;
            std::string className = getField(student, classIndex);
            if (className.empty()) {
                className = "No class";
            }
            return className;
        }
    }

    return "Class not found";
}

std::string findCourseName(const std::vector<UserAdminRecord>& courses, const std::string& courseId) {
    for (const UserAdminRecord& course : courses) {
        if (getField(course, 0) == courseId) {
            return getField(course, 1);
        }
    }

    return "Course not found";
}

int calculateCourseTuition(const std::vector<UserAdminRecord>& courses, const std::string& courseId) {
    for (const UserAdminRecord& course : courses) {
        if (getField(course, 0) == courseId) {
            const int credits = toIntSafe(getField(course, 2));
            const int feePerCredit = toIntSafe(getField(course, 5));
            return credits * feePerCredit;
        }
    }

    return 0;
}

int countEnrollmentsByCourse(const std::vector<UserAdminRecord>& enrollments, const std::string& courseId) {
    int count = 0;

    for (const UserAdminRecord& enrollment : enrollments) {
        if (getField(enrollment, 2) == courseId) {
            ++count;
        }
    }

    return count;
}

bool recordIdExists(const std::vector<UserAdminRecord>& records, const std::string& id) {
    for (const UserAdminRecord& record : records) {
        if (getField(record, 0) == id) {
            return true;
        }
    }

    return false;
}

std::string readRequiredText(const std::string& label) {
    std::string value;

    while (true) {
        std::cout << label;
        std::getline(std::cin, value);
        value = trim(value);

        if (!value.empty()) {
            return value;
        }

        std::cout << "Value cannot be empty.\n";
    }
}

bool readRequiredTextOrCancel(const std::string& label, std::string& value) {
    while (true) {
        std::cout << label << " (enter 0 to cancel): ";
        std::getline(std::cin, value);
        value = trim(value);

        if (value == "0") {
            std::cout << "Operation cancelled.\n";
            return false;
        }

        if (!value.empty()) {
            return true;
        }

        std::cout << "Value cannot be empty.\n";
    }
}

std::string readOptionalTextForUser(const std::string& label,
                                    const std::string& role,
                                    bool& hasBlankOptionalField) {
    std::string value;
    std::cout << label << " (optional, press Enter to let the "
              << role << " fill it later): ";
    std::getline(std::cin, value);
    value = trim(value);

    if (value.empty()) {
        hasBlankOptionalField = true;
        std::cout << "This information is left blank. The "
                  << role << " should update it later.\n";
    }

    return value;
}

int readPositiveNumber(const std::string& label) {
    std::string value;

    while (true) {
        std::cout << label;
        std::getline(std::cin, value);
        value = trim(value);

        try {
            const int number = std::stoi(value);
            if (number > 0) {
                return number;
            }
        } catch (...) {
        }

        std::cout << "Please enter a positive integer.\n";
    }
}

bool readPositiveNumberOrCancel(const std::string& label, int& number) {
    std::string value;

    while (true) {
        std::cout << label << " (enter 0 to cancel): ";
        std::getline(std::cin, value);
        value = trim(value);

        if (value == "0") {
            std::cout << "Operation cancelled.\n";
            return false;
        }

        try {
            number = std::stoi(value);
            if (number > 0) {
                return true;
            }
        } catch (...) {
        }

        std::cout << "Please enter a positive integer.\n";
    }
}

int toIntSafe(const std::string& value) {
    try {
        return std::stoi(trim(value));
    } catch (...) {
        return 0;
    }
}

std::string fitColumn(const std::string& value, size_t width) {
    if (value.length() <= width) {
        return value;
    }

    if (width <= 3) {
        return value.substr(0, width);
    }

    return value.substr(0, width - 3) + "...";
}

bool appendCourse(const std::string& filePath, const std::vector<std::string>& fields) {
    std::ofstream fout(filePath, std::ios::app);
    if (!fout.is_open()) {
        std::cerr << "Cannot write file: " << filePath << '\n';
        return false;
    }

    for (size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) {
            fout << " | ";
        }
        fout << fields[i];
    }
    fout << '\n';
    return true;
}

void addTeacherCourseNotification(const std::string& baseDir,
                                  const std::string& teacherId,
                                  const std::string& courseId,
                                  const std::string& courseName) {
    const std::string filePath = joinPath(baseDir, "notification_teacher.csv");
    std::ofstream fout(filePath, std::ios::app);

    if (!fout.is_open()) {
        std::cerr << "Cannot write file: " << filePath << '\n';
        return;
    }

    fout << teacherId << " | "
         << courseId << " | "
         << courseName << " | "
         << "You have been assigned to a new course"
         << '\n';
}

bool saveRecordList(const std::string& filePath, const std::vector<UserAdminRecord>& records) {
    std::ofstream fout(filePath);
    if (!fout.is_open()) {
        std::cerr << "Cannot write file: " << filePath << '\n';
        return false;
    }

    for (const UserAdminRecord& record : records) {
        for (size_t i = 0; i < record.fields.size(); ++i) {
            if (i > 0) {
                fout << " | ";
            }
            fout << record.fields[i];
        }
        fout << '\n';
    }

    return true;
}

void printCourseEnrollmentList(const std::string& baseDir) {
    const std::vector<UserAdminRecord> courses = loadAdminUserList(joinPath(baseDir, "course.csv"));
    const std::vector<UserAdminRecord> enrollments = loadAdminUserList(joinPath(baseDir, "enrollment.csv"));
    const std::vector<UserAdminRecord> teachers = loadAdminUserList(joinPath(baseDir, "teacher.csv"));

    std::cout << "\n===== COURSE AND ENROLLMENT LIST =====\n";

    if (courses.empty()) {
        std::cout << "No course data available.\n";
        return;
    }

    const int wId = 8;
    const int wName = 28;
    const int wCredits = 8;
    const int wTeacher = 28;
    const int wStudents = 12;
    const int wFee = 14;
    const int wStart = 12;
    const int wEnd = 12;
    const int wStatus = 10;
    const int totalWidth = wId + wName + wCredits + wTeacher + wStudents + wFee + wStart + wEnd + wStatus;

    std::cout << std::left
              << std::setw(wId) << "Course ID"
              << std::setw(wName) << "Course name"
              << std::setw(wCredits) << "Credits"
              << std::setw(wTeacher) << "Teacher"
              << std::setw(wStudents) << "Enrollment"
              << std::setw(wFee) << "Tuition"
              << std::setw(wStart) << "Start date"
              << std::setw(wEnd) << "End date"
              << std::setw(wStatus) << "Status"
              << '\n';
    std::cout << std::string(totalWidth, '-') << '\n';

    for (const UserAdminRecord& course : courses) {
        const std::string courseId = getField(course, 0);
        const std::string courseName = getField(course, 1);
        const std::string credits = getField(course, 2);
        const std::string teacherId = getField(course, 3);
        const int maxStudents = toIntSafe(getField(course, 4));
        const std::string feePerCredit = getField(course, 5);
        const std::string startDate = getField(course, 6);
        const std::string endDate = getField(course, 7);
        const std::string teacherName = findTeacherName(teachers, teacherId);
        const int registeredCount = countEnrollmentsByCourse(enrollments, courseId);
        const bool isClosed = maxStudents > 0 && registeredCount >= maxStudents;
        const std::string teacherDisplay = teacherName + " (" + teacherId + ")";
        const std::string studentDisplay = std::to_string(registeredCount) + "/" + std::to_string(maxStudents);

        std::cout << std::left
                  << std::setw(wId) << fitColumn(courseId, wId - 1)
                  << std::setw(wName) << fitColumn(courseName, wName - 1)
                  << std::setw(wCredits) << fitColumn(credits, wCredits - 1)
                  << std::setw(wTeacher) << fitColumn(teacherDisplay, wTeacher - 1)
                  << std::setw(wStudents) << fitColumn(studentDisplay, wStudents - 1)
                  << std::setw(wFee) << fitColumn(feePerCredit, wFee - 1)
                  << std::setw(wStart) << fitColumn(startDate, wStart - 1)
                  << std::setw(wEnd) << fitColumn(endDate, wEnd - 1)
                  << std::setw(wStatus) << (isClosed ? "Closed" : "Open")
                  << '\n';
    }
}

void printCourseDetailForEdit(const UserAdminRecord& course) {
    std::cout << "\nEditing course: " << getField(course, 1)
              << " (" << getField(course, 0) << ")\n";
    std::cout << "1. Course name: " << getField(course, 1) << '\n';
    std::cout << "2. Credits: " << getField(course, 2) << '\n';
    std::cout << "3. Teacher ID: " << getField(course, 3) << '\n';
    std::cout << "4. Maximum students: " << getField(course, 4) << '\n';
    std::cout << "5. Fee per credit: " << getField(course, 5) << '\n';
    std::cout << "6. Start date: " << getField(course, 6) << '\n';
    std::cout << "7. End date: " << getField(course, 7) << '\n';
    std::cout << "0. Save and return\n";
}

void createCourse(const std::string& baseDir) {
    const std::string coursePath = joinPath(baseDir, "course.csv");
    const std::string teacherPath = joinPath(baseDir, "teacher.csv");
    const std::vector<UserAdminRecord> courses = loadAdminUserList(coursePath);
    const std::vector<UserAdminRecord> teachers = loadAdminUserList(teacherPath);

    std::cout << "\n===== CREATE NEW COURSE =====\n";

    std::string courseId;
    while (true) {
        if (!readRequiredTextOrCancel("Course ID", courseId)) {
            return;
        }
        if (!recordIdExists(courses, courseId)) {
            break;
        }
        std::cout << "Course ID already exists. Please enter another ID.\n";
    }

    std::string courseName;
    if (!readRequiredTextOrCancel("Course name", courseName)) {
        return;
    }

    int credits = 0;
    if (!readPositiveNumberOrCancel("Credits", credits)) {
        return;
    }

    std::string teacherId;
    while (true) {
        if (!readRequiredTextOrCancel("Assigned teacher ID", teacherId)) {
            return;
        }
        if (recordIdExists(teachers, teacherId)) {
            break;
        }
        std::cout << "No teacher found with this ID.\n";
    }

    int maxStudents = 0;
    if (!readPositiveNumberOrCancel("Maximum students", maxStudents)) {
        return;
    }

    int feePerCredit = 0;
    if (!readPositiveNumberOrCancel("Fee per credit", feePerCredit)) {
        return;
    }

    std::string startDate;
    if (!readRequiredTextOrCancel("Start date (dd/mm/yyyy)", startDate)) {
        return;
    }

    std::string endDate;
    if (!readRequiredTextOrCancel("End date (dd/mm/yyyy)", endDate)) {
        return;
    }

    if (appendCourse(coursePath, {
            courseId,
            courseName,
            std::to_string(credits),
            teacherId,
            std::to_string(maxStudents),
            std::to_string(feePerCredit),
            startDate,
            endDate
        })) {
        addTeacherCourseNotification(baseDir, teacherId, courseId, courseName);
        std::cout << "Course created successfully.\n";
    }
}

void editCourse(const std::string& baseDir) {
    const std::string coursePath = joinPath(baseDir, "course.csv");
    const std::string teacherPath = joinPath(baseDir, "teacher.csv");
    std::vector<UserAdminRecord> courses = loadAdminUserList(coursePath);
    const std::vector<UserAdminRecord> teachers = loadAdminUserList(teacherPath);

    std::cout << "\n===== EDIT COURSE =====\n";
    std::string courseId;
    if (!readRequiredTextOrCancel("Enter course ID to edit", courseId)) {
        return;
    }

    for (UserAdminRecord& course : courses) {
        if (getField(course, 0) != courseId) {
            continue;
        }

        while (course.fields.size() < 8) {
            course.fields.push_back("");
        }

        const std::string oldTeacherId = getField(course, 3);

        while (true) {
            printCourseDetailForEdit(course);
            const int choice = readChoice();

            if (choice == 0) {
                if (saveRecordList(coursePath, courses)) {
                    if (getField(course, 3) != oldTeacherId) {
                        addTeacherCourseNotification(
                            baseDir,
                            getField(course, 3),
                            getField(course, 0),
                            getField(course, 1)
                        );
                    }
                    std::cout << "Course updated successfully.\n";
                }
                return;
            }

            switch (choice) {
                case 1: {
                    std::string newValue;
                    if (readRequiredTextOrCancel("New course name", newValue)) {
                        course.fields[1] = newValue;
                    }
                    break;
                }
                case 2: {
                    int newValue = 0;
                    if (readPositiveNumberOrCancel("New credits", newValue)) {
                        course.fields[2] = std::to_string(newValue);
                    }
                    break;
                }
                case 3: {
                    std::string newTeacherId;
                    while (true) {
                        if (!readRequiredTextOrCancel("New teacher ID", newTeacherId)) {
                            break;
                        }
                        if (recordIdExists(teachers, newTeacherId)) {
                            course.fields[3] = newTeacherId;
                            break;
                        }
                        std::cout << "No teacher found with this ID.\n";
                    }
                    break;
                }
                case 4: {
                    int newValue = 0;
                    if (readPositiveNumberOrCancel("New maximum students", newValue)) {
                        course.fields[4] = std::to_string(newValue);
                    }
                    break;
                }
                case 5: {
                    int newValue = 0;
                    if (readPositiveNumberOrCancel("New fee per credit", newValue)) {
                        course.fields[5] = std::to_string(newValue);
                    }
                    break;
                }
                case 6: {
                    std::string newValue;
                    if (readRequiredTextOrCancel("New start date (dd/mm/yyyy)", newValue)) {
                        course.fields[6] = newValue;
                    }
                    break;
                }
                case 7: {
                    std::string newValue;
                    if (readRequiredTextOrCancel("New end date (dd/mm/yyyy)", newValue)) {
                        course.fields[7] = newValue;
                    }
                    break;
                }
                default:
                    std::cout << "Invalid choice.\n";
                    break;
            }
        }
    }

    std::cout << "No course found with ID: " << courseId << '\n';
}

void runCourseMenu(const std::string& baseDir) {
    while (true) {
        std::cout << "\n===== COURSE MANAGEMENT =====\n";
        std::cout << "1. View courses and enrollment information\n";
        std::cout << "2. Create new course\n";
        std::cout << "3. Edit course\n";
        std::cout << "0. Back to admin menu\n";
        std::cout << "9. Exit program\n";

        switch (readChoice()) {
            case 1:
                printCourseEnrollmentList(baseDir);
                pauseScreen();
                break;
            case 2:
                createCourse(baseDir);
                pauseScreen();
                break;
            case 3:
                editCourse(baseDir);
                pauseScreen();
                break;
            case 0:
                return;
            case 9:
                exitProgram();
            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
        }
    }
}

void printTuitionList(const std::string& baseDir) {
    const std::vector<UserAdminRecord> enrollments = loadAdminUserList(joinPath(baseDir, "enrollment.csv"));
    const std::vector<UserAdminRecord> students = loadAdminUserList(joinPath(baseDir, "student.csv"));
    const std::vector<UserAdminRecord> courses = loadAdminUserList(joinPath(baseDir, "course.csv"));

    std::cout << "\n===== TUITION LIST =====\n";

    if (enrollments.empty()) {
        std::cout << "No tuition enrollment data available.\n";
        return;
    }

    const int wEnroll = 10;
    const int wStudentId = 12;
    const int wStudent = 24;
    const int wCourseId = 10;
    const int wCourse = 28;
    const int wDate = 14;
    const int wTuition = 14;
    const int wStatus = 14;
    const int totalWidth = wEnroll + wStudentId + wStudent + wCourseId + wCourse + wDate + wTuition + wStatus;

    long long totalTuition = 0;
    long long paidTuition = 0;
    long long unpaidTuition = 0;
    std::vector<std::string> classes;

    for (const UserAdminRecord& enrollment : enrollments) {
        const std::string studentClass = findStudentClass(students, getField(enrollment, 1));
        bool exists = false;

        for (const std::string& currentClass : classes) {
            if (currentClass == studentClass) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            classes.push_back(studentClass);
        }
    }

    for (const std::string& className : classes) {
        long long classTotal = 0;
        long long classPaid = 0;
        long long classUnpaid = 0;

        std::cout << "\n--- Class: " << className << " ---\n";
        std::cout << std::left
                  << std::setw(wEnroll) << "Enrollment ID"
                  << std::setw(wStudentId) << "Student ID"
                  << std::setw(wStudent) << "Student"
                  << std::setw(wCourseId) << "Course ID"
                  << std::setw(wCourse) << "Course"
                  << std::setw(wDate) << "Enroll date"
                  << std::setw(wTuition) << "Tuition"
                  << std::setw(wStatus) << "Status"
                  << '\n';
        std::cout << std::string(totalWidth, '-') << '\n';

        for (const UserAdminRecord& enrollment : enrollments) {
            if (findStudentClass(students, getField(enrollment, 1)) != className) {
                continue;
            }

        const std::string enrollId = getField(enrollment, 0);
        const std::string studentId = getField(enrollment, 1);
        const std::string courseId = getField(enrollment, 2);
        const std::string enrollDate = getField(enrollment, 3);
        const std::string studentName = findStudentName(students, studentId);
        const std::string courseName = findCourseName(courses, courseId);
        const std::string paymentStatus = getField(enrollment, 8).empty() ? "Unpaid" : getField(enrollment, 8);

        int tuition = toIntSafe(getField(enrollment, 7));
        if (tuition <= 0) {
            tuition = calculateCourseTuition(courses, courseId);
        }

        totalTuition += tuition;
        if (paymentStatus == "Paid" || paymentStatus == "paid") {
            paidTuition += tuition;
            classPaid += tuition;
        } else {
            unpaidTuition += tuition;
            classUnpaid += tuition;
        }
        classTotal += tuition;

        std::cout << std::left
                  << std::setw(wEnroll) << fitColumn(enrollId, wEnroll - 1)
                  << std::setw(wStudentId) << fitColumn(studentId, wStudentId - 1)
                  << std::setw(wStudent) << fitColumn(studentName, wStudent - 1)
                  << std::setw(wCourseId) << fitColumn(courseId, wCourseId - 1)
                  << std::setw(wCourse) << fitColumn(courseName, wCourse - 1)
                  << std::setw(wDate) << fitColumn(enrollDate, wDate - 1)
                  << std::setw(wTuition) << tuition
                  << std::setw(wStatus) << fitColumn(paymentStatus, wStatus - 1)
                  << '\n';
        }

        std::cout << std::string(totalWidth, '-') << '\n';
        std::cout << "Class total " << className << ": " << classTotal
                  << " | Paid: " << classPaid
                  << " | Unpaid: " << classUnpaid << '\n';
    }

    std::cout << "\nTotal tuition for all classes: " << totalTuition
              << " | Paid: " << paidTuition
              << " | Unpaid: " << unpaidTuition << '\n';
}

void updatePaymentStatus(const std::string& baseDir) {
    const std::string enrollmentPath = joinPath(baseDir, "enrollment.csv");
    std::vector<UserAdminRecord> enrollments = loadAdminUserList(enrollmentPath);

    std::cout << "\n===== UPDATE TUITION PAYMENT STATUS =====\n";
    std::string enrollId;
    if (!readRequiredTextOrCancel("Enter enrollment ID", enrollId)) {
        return;
    }

    for (UserAdminRecord& enrollment : enrollments) {
        if (getField(enrollment, 0) != enrollId) {
            continue;
        }

        while (enrollment.fields.size() < 9) {
            enrollment.fields.push_back("");
        }

        std::cout << "1. Paid\n";
        std::cout << "2. Unpaid\n";
        const int choice = readChoice();

        if (choice == 1) {
            enrollment.fields[8] = "Paid";
        } else if (choice == 2) {
            enrollment.fields[8] = "Unpaid";
        } else {
            std::cout << "Invalid choice.\n";
            return;
        }

        if (saveRecordList(enrollmentPath, enrollments)) {
            std::cout << "Tuition payment status updated successfully.\n";
        }
        return;
    }

    std::cout << "Enrollment ID not found: " << enrollId << '\n';
}

void runPaymentMenu(const std::string& baseDir) {
    while (true) {
        std::cout << "\n===== TUITION MANAGEMENT =====\n";
        std::cout << "1. View tuition list\n";
        std::cout << "2. Update payment status\n";
        std::cout << "0. Back to admin menu\n";
        std::cout << "9. Exit program\n";

        switch (readChoice()) {
            case 1:
                printTuitionList(baseDir);
                pauseScreen();
                break;
            case 2:
                updatePaymentStatus(baseDir);
                pauseScreen();
                break;
            case 0:
                return;
            case 9:
                exitProgram();
            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
        }
    }
}
}  // namespace

std::vector<UserAdminRecord> loadAdminUserList(const std::string& filePath) {
    std::ifstream fin(filePath);
    std::vector<UserAdminRecord> records;
    std::string line;

    if (!fin.is_open()) {
        std::cerr << "Cannot open file: " << filePath << '\n';
        return records;
    }

    while (std::getline(fin, line)) {
        if (trim(line).empty()) {
            continue;
        }

        UserAdminRecord record;
        record.fields = splitByPipe(line);
        records.push_back(record);
    }

    return records;
}

void printUserList(const std::string& title, const std::vector<UserAdminRecord>& records) {
    std::cout << "\n=== " << title << " (" << records.size() << " records) ===\n";

    for (size_t i = 0; i < records.size(); ++i) {
        std::cout << std::setw(2) << (i + 1) << ". ";
        for (size_t j = 0; j < records[i].fields.size(); ++j) {
            if (j > 0) {
                std::cout << " | ";
            }
            std::cout << records[i].fields[j];
        }
        std::cout << '\n';
    }
}

size_t studentInfoIndex(const UserAdminRecord& student, size_t newIndex) {
    if (student.fields.size() >= 10 && student.fields.back() == "student") {
        return newIndex;
    }

    if (newIndex >= 4) {
        return newIndex - 1;
    }

    return newIndex;
}

std::string getStudentMajor(const UserAdminRecord& student) {
    std::string major = getField(student, studentInfoIndex(student, 7));
    if (major.empty()) {
        major = "No major";
    }
    return major;
}

void printStudentListByMajor(const std::vector<UserAdminRecord>& students) {
    std::vector<std::string> majors;

    for (const UserAdminRecord& student : students) {
        const std::string major = getStudentMajor(student);
        bool exists = false;

        for (const std::string& currentMajor : majors) {
            if (currentMajor == major) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            majors.push_back(major);
        }
    }

    std::cout << "\n=== STUDENT LIST BY MAJOR (" << students.size() << " records) ===\n";

    if (students.empty()) {
        std::cout << "No student data available.\n";
        return;
    }

    const int wNo = 5;
    const int wId = 13;
    const int wName = 24;
    const int wEmail = 30;
    const int wDob = 14;
    const int wPhone = 14;
    const int wClass = 12;
    const int totalWidth = wNo + wId + wName + wEmail + wDob + wPhone + wClass;

    for (const std::string& major : majors) {
        int count = 0;
        for (const UserAdminRecord& student : students) {
            if (getStudentMajor(student) == major) {
                ++count;
            }
        }

        std::cout << "\n--- Major/Department: " << major << " (" << count << " students) ---\n";
        std::cout << std::left
                  << std::setw(wNo) << "No."
                  << std::setw(wId) << "Student ID"
                  << std::setw(wName) << "Full name"
                  << std::setw(wEmail) << "Email"
                  << std::setw(wDob) << "Date of birth"
                  << std::setw(wPhone) << "Phone"
                  << std::setw(wClass) << "Class"
                  << '\n';
        std::cout << std::string(totalWidth, '-') << '\n';

        int index = 1;
        for (const UserAdminRecord& student : students) {
            if (getStudentMajor(student) != major) {
                continue;
            }

            std::cout << std::left
                      << std::setw(wNo) << index
                      << std::setw(wId) << fitColumn(getField(student, 0), wId - 1)
                      << std::setw(wName) << fitColumn(getField(student, 1), wName - 1)
                      << std::setw(wEmail) << fitColumn(getField(student, 2), wEmail - 1)
                      << std::setw(wDob) << fitColumn(getField(student, studentInfoIndex(student, 4)), wDob - 1)
                      << std::setw(wPhone) << fitColumn(getField(student, studentInfoIndex(student, 5)), wPhone - 1)
                      << std::setw(wClass) << fitColumn(getField(student, studentInfoIndex(student, 8)), wClass - 1)
                      << '\n';
            ++index;
        }
    }
}

size_t teacherInfoIndex(const UserAdminRecord& teacher, size_t newIndex) {
    if (teacher.fields.size() >= 9 && teacher.fields.back() == "teacher") {
        if (newIndex == 4) return 5;
        if (newIndex == 5) return 4;
        return newIndex;
    }

    if (newIndex == 4) return teacher.fields.size();
    if (newIndex >= 5) return newIndex - 1;
    return newIndex;
}

bool isDateText(const std::string& value) {
    return value.size() == 10 &&
           value[2] == '/' &&
           value[5] == '/';
}

size_t teacherPhoneIndex(const UserAdminRecord& teacher) {
    return 4;
}

std::string getTeacherDepartment(const UserAdminRecord& teacher) {
    std::string department = getField(teacher, teacherInfoIndex(teacher, 7));
    if (department.empty()) {
        department = "No department";
    }
    return department;
}

void printTeacherListByDepartment(const std::vector<UserAdminRecord>& teachers) {
    std::vector<std::string> departments;

    for (const UserAdminRecord& teacher : teachers) {
        const std::string department = getTeacherDepartment(teacher);
        bool exists = false;

        for (const std::string& currentDepartment : departments) {
            if (currentDepartment == department) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            departments.push_back(department);
        }
    }

    std::cout << "\n=== TEACHER LIST BY DEPARTMENT (" << teachers.size() << " records) ===\n";

    if (teachers.empty()) {
        std::cout << "No teacher data available.\n";
        return;
    }

    const int wNo = 5;
    const int wId = 13;
    const int wName = 24;
    const int wEmail = 30;
    const int wDob = 14;
    const int wPhone = 14;
    const int wAddress = 14;
    const int totalWidth = wNo + wId + wName + wEmail + wDob + wPhone + wAddress;

    for (const std::string& department : departments) {
        int count = 0;
        for (const UserAdminRecord& teacher : teachers) {
            if (getTeacherDepartment(teacher) == department) {
                ++count;
            }
        }

        std::cout << "\n--- Department/Major: " << department << " (" << count << " teachers) ---\n";
        std::cout << std::left
                  << std::setw(wNo) << "No."
                  << std::setw(wId) << "Teacher ID"
                  << std::setw(wName) << "Full name"
                  << std::setw(wEmail) << "Email"
                  << std::setw(wDob) << "Date of birth"
                  << std::setw(wPhone) << "Phone"
                  << std::setw(wAddress) << "Address"
                  << '\n';
        std::cout << std::string(totalWidth, '-') << '\n';

        int index = 1;
        for (const UserAdminRecord& teacher : teachers) {
            if (getTeacherDepartment(teacher) != department) {
                continue;
            }

            std::cout << std::left
                      << std::setw(wNo) << index
                      << std::setw(wId) << fitColumn(getField(teacher, 0), wId - 1)
                      << std::setw(wName) << fitColumn(getField(teacher, 1), wName - 1)
                      << std::setw(wEmail) << fitColumn(getField(teacher, 2), wEmail - 1)
                      << std::setw(wDob) << fitColumn(getField(teacher, teacherInfoIndex(teacher, 4)), wDob - 1)
                      << std::setw(wPhone) << fitColumn(getField(teacher, teacherPhoneIndex(teacher)), wPhone - 1)
                      << std::setw(wAddress) << fitColumn(getField(teacher, teacherInfoIndex(teacher, 6)), wAddress - 1)
                      << '\n';
            ++index;
        }
    }
}

bool saveAdminUserList(const std::string& filePath, const std::vector<UserAdminRecord>& records) {
    std::ofstream fout(filePath);
    if (!fout.is_open()) {
        std::cerr << "Cannot write file: " << filePath << '\n';
        return false;
    }

    for (const UserAdminRecord& record : records) {
        for (size_t i = 0; i < record.fields.size(); ++i) {
            if (i > 0) {
                fout << " | ";
            }
            fout << record.fields[i];
        }
        fout << '\n';
    }

    return true;
}

void addUserRecord(const std::string& filePath,
                   const std::string& role,
                   const std::vector<std::string>& fieldLabels) {
    std::vector<UserAdminRecord> records = loadAdminUserList(filePath);
    UserAdminRecord newRecord;

    std::cout << "\n===== ADD " << role << " =====\n";

    std::string id;
    while (true) {
        if (!readRequiredTextOrCancel(fieldLabels[0], id)) {
            return;
        }
        if (!recordIdExists(records, id)) {
            break;
        }
        std::cout << "ID already exists. Please enter another ID.\n";
    }
    newRecord.fields.push_back(id);

    bool hasBlankOptionalField = false;
    for (size_t i = 1; i < fieldLabels.size(); ++i) {
        std::string value;
        if (i <= 3) {
            if (!readRequiredTextOrCancel(fieldLabels[i], value)) {
                return;
            }
        } else {
            value = readOptionalTextForUser(fieldLabels[i], role, hasBlankOptionalField);
        }
        newRecord.fields.push_back(value);
    }
    newRecord.fields.push_back(role);

    records.push_back(newRecord);
    if (saveAdminUserList(filePath, records)) {
        std::cout << "Add " << role << " successfully.\n";
        if (hasBlankOptionalField) {
            std::cout << "Some optional information was left blank. The "
                      << role << " can fill it in later.\n";
        }
    }
}

void ensureDateOfBirthField(UserAdminRecord& record, const std::string& role) {
    const size_t legacySize = (role == "student") ? 9 : 8;

    if (record.fields.size() == legacySize && record.fields.back() == role) {
        const size_t insertIndex = (role == "teacher") ? 5 : 4;
        record.fields.insert(record.fields.begin() + insertIndex, "");
    }
}

void deleteUserRecord(const std::string& filePath, const std::string& role) {
    std::vector<UserAdminRecord> records = loadAdminUserList(filePath);

    std::cout << "\n===== DELETE " << role << " =====\n";
    std::string id;
    if (!readRequiredTextOrCancel("Enter ID to delete", id)) {
        return;
    }

    for (size_t i = 0; i < records.size(); ++i) {
        if (getField(records[i], 0) == id) {
            std::cout << "Found: " << getField(records[i], 1) << '\n';
            std::string confirm;
            if (!readRequiredTextOrCancel("Enter y to confirm deletion", confirm)) {
                return;
            }

            if (confirm == "y" || confirm == "Y") {
                records.erase(records.begin() + i);
                if (saveAdminUserList(filePath, records)) {
                    std::cout << "Delete " << role << " successfully.\n";
                }
            } else {
                std::cout << "Delete cancelled.\n";
            }
            return;
        }
    }

    std::cout << "ID not found: " << id << '\n';
}

void editUserRecord(const std::string& filePath,
                    const std::string& role,
                    const std::vector<std::string>& fieldLabels) {
    std::vector<UserAdminRecord> records = loadAdminUserList(filePath);

    std::cout << "\n===== EDIT " << role << " =====\n";
    std::string id;
    if (!readRequiredTextOrCancel("Enter ID to edit", id)) {
        return;
    }

    for (UserAdminRecord& record : records) {
        if (getField(record, 0) != id) {
            continue;
        }

        ensureDateOfBirthField(record, role);

        while (true) {
            std::cout << "\nEditing: " << getField(record, 1) << " (" << id << ")\n";
            for (size_t i = 1; i < fieldLabels.size(); ++i) {
                std::cout << i << ". " << fieldLabels[i]
                          << " current: " << getField(record, i) << '\n';
            }
            std::cout << "0. Save and return\n";

            const int choice = readChoice();
            if (choice == 0) {
                if (saveAdminUserList(filePath, records)) {
                    std::cout << "Updated " << role << " successfully.\n";
                }
                return;
            }

            if (choice < 1 || static_cast<size_t>(choice) >= fieldLabels.size()) {
                std::cout << "Invalid choice.\n";
                continue;
            }

            while (record.fields.size() <= static_cast<size_t>(choice)) {
                record.fields.push_back("");
            }
            std::string newValue;
            if (readRequiredTextOrCancel("New value", newValue)) {
                record.fields[choice] = newValue;
            }
        }
    }

    std::cout << "ID not found: " << id << '\n';
}

void runUserListMenu(const std::string& baseDir) {
    const std::vector<UserAdminRecord> students = loadAdminUserList(joinPath(baseDir, "student.csv"));
    const std::vector<UserAdminRecord> teachers = loadAdminUserList(joinPath(baseDir, "teacher.csv"));
    const std::vector<UserAdminRecord> admins = loadAdminUserList(joinPath(baseDir, "admin.csv"));

    int choice = -1;
    while (true) {
        std::cout << "\n===== LIST MENU =====\n";
        std::cout << "1. Print student list\n";
        std::cout << "2. Print teacher list\n";
        std::cout << "3. Print admin list\n";
        std::cout << "4. Print all lists\n";
        std::cout << "0. Exit\n";
        std::cout << "9. Exit program\n";
        std::cout << "Enter your choice: ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid choice. Please try again.\n";
            continue;
        }

        switch (choice) {
            case 1:
                printUserList("Student list", students);
                break;
            case 2:
                printUserList("Teacher list", teachers);
                break;
            case 3:
                printUserList("Admin list", admins);
                break;
            case 4:
                printUserList("Student list", students);
                printUserList("Teacher list", teachers);
                printUserList("Admin list", admins);
                break;
            case 0:
                std::cout << "Exited list menu.\n";
                return;
            case 9:
                exitProgram();
            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
        }
    }
}

void runStudentMenu(const std::string& baseDir) {
    const std::string studentPath = joinPath(baseDir, "student.csv");
    const std::vector<std::string> studentFields = {
        "ID",
        "Full name",
        "Email",
        "Password",
        "Date of birth",
        "Phone",
        "Address",
        "Major",
        "Class"
    };

    while (true) {
        std::cout << "\n===== STUDENT MANAGEMENT =====\n";
        std::cout << "1. View student list\n";
        std::cout << "2. Add student\n";
        std::cout << "3. Edit student information\n";
        std::cout << "4. Delete student\n";
        std::cout << "0. Back\n";
        std::cout << "9. Exit program\n";

        switch (readChoice()) {
            case 1:
                printStudentListByMajor(loadAdminUserList(studentPath));
                pauseScreen();
                break;
            case 2:
                addUserRecord(studentPath, "student", studentFields);
                pauseScreen();
                break;
            case 3:
                editUserRecord(studentPath, "student", studentFields);
                pauseScreen();
                break;
            case 4:
                deleteUserRecord(studentPath, "student");
                pauseScreen();
                break;
            case 0:
                return;
            case 9:
                exitProgram();
            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
        }
    }
}

void runTeacherMenu(const std::string& baseDir) {
    const std::string teacherPath = joinPath(baseDir, "teacher.csv");
    const std::vector<std::string> teacherFields = {
        "ID",
        "Full name",
        "Email",
        "Password",
        "Phone",
        "Date of birth",
        "Address",
        "Department"
    };

    while (true) {
        std::cout << "\n===== TEACHER MANAGEMENT =====\n";
        std::cout << "1. View teacher list\n";
        std::cout << "2. Add teacher\n";
        std::cout << "3. Edit teacher information\n";
        std::cout << "4. Delete teacher\n";
        std::cout << "0. Back\n";
        std::cout << "9. Exit program\n";

        switch (readChoice()) {
            case 1:
                printTeacherListByDepartment(loadAdminUserList(teacherPath));
                pauseScreen();
                break;
            case 2:
                addUserRecord(teacherPath, "teacher", teacherFields);
                pauseScreen();
                break;
            case 3:
                editUserRecord(teacherPath, "teacher", teacherFields);
                pauseScreen();
                break;
            case 4:
                deleteUserRecord(teacherPath, "teacher");
                pauseScreen();
                break;
            case 0:
                return;
            case 9:
                exitProgram();
            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
        }
    }
}

void runAdminMenu(const std::string& baseDir) {
    while (true) {
        std::cout << "\n========== ADMIN MENU ==========\n";
        std::cout << "1. Manage students\n";
        std::cout << "2. Manage teachers\n";
        std::cout << "3. Manage courses\n";
        std::cout << "4. Manage tuition\n";
        std::cout << "0. Log out\n";
        std::cout << "9. Exit program\n";

        switch (readChoice()) {
            case 1:
                runStudentMenu(baseDir);
                break;
            case 2:
                runTeacherMenu(baseDir);
                break;
            case 3:
                runCourseMenu(baseDir);
                break;
            case 4:
                runPaymentMenu(baseDir);
                break;
            case 0:
                return;
            case 9:
                exitProgram();
            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
        }
    }
}
