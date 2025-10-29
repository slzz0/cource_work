#include "student.h"
#include <sstream>
#include <iomanip>

Student::Student(const std::string& name, const std::string& surname,
                 int course, int semester, double averageGrade, bool isBudget)
    : name(name), surname(surname), course(course),
      semester(semester), averageGrade(averageGrade), isBudget(isBudget) {
}

double Student::calculateAverageGrade() const {
    return averageGrade;
}

std::string Student::getFullName() const {
    return surname + " " + name;
}

std::string Student::getStudentInfo() const {
    std::ostringstream oss;
    oss << "Student: " << getFullName() << "\n"
        << "Course: " << course << ", Semester: " << semester << "\n"
        << "Average Grade: " << std::fixed << std::setprecision(2) 
        << calculateAverageGrade();
    return oss.str();
}

// no-op: grades are not stored individually anymore

