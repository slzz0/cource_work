#include "student.h"

#include <iomanip>
#include <sstream>

#include "exceptions.h"
#include "scholarshipCalculator.h"

Student::Student(const std::string& name, const std::string& surname, int course, int semester,
                 double averageGrade, bool isBudget)
    : name(name),
      surname(surname),
      course(course),
      semester(semester),
      averageGrade(averageGrade),
      isBudget(isBudget) {
    if (name.empty() || surname.empty()) {
        throw EmptyNameException();
    }

    if (course < 1 || course > 4) {
        throw InvalidCourseException(course);
    }

    if (semester < 1 || semester > 8) {
        throw InvalidSemesterException(semester);
    }

    if (averageGrade < 0.0 || averageGrade > 10.0) {
        throw InvalidGradeException(averageGrade);
    }

    scholarship = 0.0;
}

double Student::calculateAverageGrade() const { return averageGrade; }

std::string Student::getFullName() const { return surname + " " + name; }

std::string Student::getStudentInfo() const {
    std::ostringstream oss;
    oss << "Student: " << getFullName() << "\n"
        << "Course: " << course << ", Semester: " << semester << "\n"
        << "Average Grade: " << std::fixed << std::setprecision(2) << calculateAverageGrade();
    return oss.str();
}

void Student::recalculateScholarship() {
    scholarship = 0.0;

    if (!isBudget) {
        return;
    }

    if (missedHours >= 12) {
        if (hasSocialScholarship) {
            scholarship = ScholarshipCalculator::SOCIAL_SCHOLARSHIP;
        }
        return;
    }

    scholarship = ScholarshipCalculator::calculateScholarship(averageGrade);

    if (hasSocialScholarship) {
        scholarship += ScholarshipCalculator::SOCIAL_SCHOLARSHIP;
    }
}

std::string Student::getHistoryString() const {
    if (previousSemesterGrades.empty()) {
        return "";
    }

    std::ostringstream oss;
    bool first = true;
    for (const auto& pair : previousSemesterGrades) {
        if (!first) oss << ";";
        oss << pair.first << ":" << std::fixed << std::setprecision(1) << pair.second;
        first = false;
    }
    return oss.str();
}