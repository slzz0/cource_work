#include "studentDatabase.h"
#include <algorithm>
#include <cctype>
#include <string>
#include <functional>
#include <cmath>

void StudentDatabase::addStudent(std::shared_ptr<Student> student) {
    if (student != nullptr) {
        students.push_back(student);
    }
}

void StudentDatabase::addStudent(const std::string& name, const std::string& surname,
                                 int course, int semester, double averageGrade, bool isBudget) {
    auto student = std::make_shared<Student>(name, surname, course, semester, averageGrade, isBudget);
    students.push_back(student);
}

bool StudentDatabase::removeStudent(const std::string& name, const std::string& surname) {
    auto it = std::remove_if(students.begin(), students.end(),
        [&name, &surname](const std::shared_ptr<Student>& student) {
            return student->getName() == name && student->getSurname() == surname;
        });
    
    if (it != students.end()) {
        students.erase(it, students.end());
        return true;
    }
    return false;
}

bool StudentDatabase::removeStudent(size_t index) {
    if (index < students.size()) {
        students.erase(students.begin() + index);
        return true;
    }
    return false;
}

bool StudentDatabase::removeStudentPtr(const std::shared_ptr<Student>& studentPtr) {
    auto it = std::find(students.begin(), students.end(), studentPtr);
    if (it != students.end()) {
        students.erase(it);
        return true;
    }
    return false;
}

std::shared_ptr<Student> StudentDatabase::getStudent(size_t index) const {
    if (index < students.size()) {
        return students[index];
    }
    return nullptr;
}

// Helper function to convert string to lowercase
std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::vector<std::shared_ptr<Student>> StudentDatabase::searchByName(const std::string& name) const {
    std::string lowerName = toLower(name);
    return searchStudents([&lowerName](const std::shared_ptr<Student>& student) {
        return toLower(student->getName()).find(lowerName) != std::string::npos ||
               toLower(student->getFullName()).find(lowerName) != std::string::npos;
    });
}

std::vector<std::shared_ptr<Student>> StudentDatabase::searchBySurname(const std::string& surname) const {
    std::string lowerSurname = toLower(surname);
    return searchStudents([&lowerSurname](const std::shared_ptr<Student>& student) {
        return toLower(student->getSurname()).find(lowerSurname) != std::string::npos;
    });
}

std::vector<std::shared_ptr<Student>> StudentDatabase::searchByAverageGrade(double minGrade, double maxGrade) const {
    return searchStudents([minGrade, maxGrade](const std::shared_ptr<Student>& student) {
        double avg = student->calculateAverageGrade();
        return avg >= minGrade && avg <= maxGrade;
    });
}

std::vector<std::shared_ptr<Student>> StudentDatabase::searchByAverageAtLeast(double minGrade) const {
    return searchStudents([minGrade](const std::shared_ptr<Student>& student) {
        return student->calculateAverageGrade() >= minGrade;
    });
}

std::vector<std::shared_ptr<Student>> StudentDatabase::searchByAverageEqual(double exactGrade) const {
    auto round2 = [](double v) {
        return std::round(v * 100.0) / 100.0;
    };
    double target = round2(exactGrade);
    return searchStudents([target, round2](const std::shared_ptr<Student>& student) {
        return round2(student->calculateAverageGrade()) == target;
    });
}

std::vector<std::shared_ptr<Student>> StudentDatabase::searchByCourse(int course) const {
    return searchStudents([course](const std::shared_ptr<Student>& student) {
        return student->getCourse() == course;
    });
}

void StudentDatabase::clear() {
    students.clear();
}

