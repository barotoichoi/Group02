#include <iostream>
#include "student.h"

using namespace std;

string getCsvFilePath() {
    
    return "student .csv";
}

int main() {
    cout << "===== DANG NHAP SINH VIEN =====\n";
    cout << "File du lieu se duoc doc tu CSV.\n";

    string filePath = getCsvFilePath();
    cout << "Su dung file: " << filePath << "\n\n";

    
    runLogin(filePath);

    cout << "\n Cam on ban!\n";
    return 0;
}
