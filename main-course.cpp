#include <string>
#include <iostream>

using namespace std;

// Prototype de tranh undefined reference khi build tach file2
void studentCourseMenu(const string& studentId);

int main() {
    string studentId;
    cout << "Nhap student_id: ";
    getline(cin, studentId);

    size_t start = studentId.find_first_not_of(" \t\r\n");
    size_t end = studentId.find_last_not_of(" \t\r\n");
    if (start == string::npos) {
        cout << "student_id khong hop le.\n";
        return 0;
    }
    studentId = studentId.substr(start, end - start + 1);

    studentCourseMenu(studentId);
    return 0;
}
