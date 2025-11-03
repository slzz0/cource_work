#include "student.h"
#include "exceptions.h"

#include <iomanip>
#include <sstream>

#include "scholarshipCalculator.h"

Student::Student(const std::string& name, const std::string& surname, int course, int semester,
                 double averageGrade, bool isBudget)
    : name(name),
      surname(surname),
      course(course),
      semester(semester),
      averageGrade(averageGrade),
      isBudget(isBudget) {
    
    // Validate input data
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
    
    recalculateScholarship();
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
        return;  // Платники не получают стипендию
    }

    // Если пропущено 12 или больше часов - лишаем стипендии
    if (missedHours >= 12) {
        return;
    }

    // Базовая стипендия по среднему баллу
    scholarship = ScholarshipCalculator::calculateScholarship(averageGrade);

    // Добавляем социальную стипендию если есть
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