#include "student.h"
#include <iomanip>
#include <sstream>
#include "scholarshipCalculator.h"

Student::Student(const std::string &name, const std::string &surname,
                 int course, int semester, double averageGrade, bool isBudget)
    : name(name), surname(surname), course(course), semester(semester),
      averageGrade(averageGrade), isBudget(isBudget) {
  recalculateScholarship();
}

double Student::calculateAverageGrade() const { return averageGrade; }

std::string Student::getFullName() const { return surname + " " + name; }

std::string Student::getStudentInfo() const {
  std::ostringstream oss;
  oss << "Student: " << getFullName() << "\n"
      << "Course: " << course << ", Semester: " << semester << "\n"
      << "Average Grade: " << std::fixed << std::setprecision(2)
      << calculateAverageGrade();
  return oss.str();
}

void Student::recalculateScholarship() {
  if (isBudget) {
    scholarship = ScholarshipCalculator::calculateScholarship(averageGrade);
  } else {
    scholarship = 0.0;
  }
}
