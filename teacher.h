#ifndef TEACHER_H
#define TEACHER_H

#include <vector>
#include <string>

struct CsvRecord {
    std::vector<std::string> fields;
};

std::string joinPath(const std::string& dir, const std::string& fileName);
std::string trimString(const std::string& value);
std::vector<CsvRecord> loadCsv(const std::string& filePath);
void saveCsv(const std::string& filePath, const std::vector<CsvRecord>& records);

void updatePersonalInfo(const std::string& baseDir, const std::string& teacherId);
void viewTeachingSchedule(const std::string& baseDir, const std::string& teacherId);
void viewStudentList(const std::string& baseDir, const std::string& teacherId);
void inputGrades(const std::string& baseDir, const std::string& teacherId);
void viewCourseList(const std::string& baseDir, const std::string& teacherId);
void changePassword(const std::string& baseDir, const std::string& teacherId);

void runTeacherMenu(const std::string& baseDir, const std::string& teacherId);
void login(const std::string& baseDir);

#endif
