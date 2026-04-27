#ifndef STUDENT_H
#define STUDENT_H

#include <string>
#include <vector>

struct Student {
    std::vector<std::string> fields;
};

std::vector<Student> loadStudents(const std::string& filePath);
void runLogin(const std::string& filePath);

#endif
