#ifndef STUDENT_H
#define STUDENT_H

#include <string>
#include <vector>
using namespace std;

struct UserStudentRecord {
    std::vector<std::string> fields;
};

std::vector<UserStudentRecord> loadUserList(const std::string& filePath);
void runStudentMenu(const std::string& baseDir, const std::string& studentId);

#endif
