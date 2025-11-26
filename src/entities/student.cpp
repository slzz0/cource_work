#include "entities/student.h"

#include <format>
#include <iomanip>
#include <sstream>

#include "exceptions/exceptions.h"
#include "services/scholarshipCalculator.h"

Student::Student(std::string_view name, std::string_view surname, int course, int semester,
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
    if (isBudget) {
        budgetSemester = 1;
    }
}

double Student::calculateAverageGrade() const { return averageGrade; }

std::string Student::getFullName() const { return surname + " " + name; }

std::string Student::getStudentInfo() const {
    return std::format("Student: {}\nCourse: {}, Semester: {}\nAverage Grade: {:.2f}",
                       getFullName(), course, semester, calculateAverageGrade());
}

void Student::recalculateScholarship() {
    saveHistoricalScholarships();
    
    scholarship = 0.0;

    if (!isBudget) {
        return;
    }

    if (!isEligibleForScholarship()) {
        return;
    }

    calculateCurrentScholarship();
    saveCurrentScholarshipToHistory();
}

void Student::saveHistoricalScholarships() {
    if (!isBudget || budgetSemester <= 0) {
        return;
    }
    
    saveScholarshipsForBudgetSemesters(budgetSemester);
}

bool Student::isEligibleForScholarship() const {
    if (!isBudget) {
        return false;
    }
    if (budgetSemester > 0 && semester < budgetSemester) {
        return false;  
    }
    return true;
}

void Student::calculateCurrentScholarship() {
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

void Student::saveCurrentScholarshipToHistory() {
    if (scholarship > 0.0) {
        previousSemesterScholarships[semester] = scholarship;
    }
}

void Student::setSemester(int newSemester) {
    if (isBudget && scholarship > 0.0 && semester > 0 && budgetSemester > 0 && semester >= budgetSemester) {
        previousSemesterScholarships[semester] = scholarship;
    }
    this->semester = newSemester;
    this->course = (semester - 1) / 2 + 1;
}

void Student::setIsBudget(bool newIsBudget) {

    if (isBudget == newIsBudget) {
        return;
    }
    
    bool wasBudget = isBudget;
    isBudget = newIsBudget;
    
    if (wasBudget && !newIsBudget) {
        scholarship = 0.0;
        previousSemesterScholarships.erase(semester);
    }
    else if (!wasBudget && newIsBudget) {
        budgetSemester = semester;
        std::erase_if(previousSemesterScholarships,
                      [this](const auto& pair) { return pair.first < budgetSemester; });
        if (isEligibleForScholarship()) {
            calculateCurrentScholarship();
            saveCurrentScholarshipToHistory();
        } else {
            scholarship = 0.0;
        }
    }
}

void Student::saveScholarshipsForBudgetSemesters(int startSemester) {
    for (const auto& [sem, grade] : previousSemesterGrades) {
        if (sem < startSemester) {
            continue;
        }
        
        if (sem == semester) {
            continue;
        }
        
        if (previousSemesterScholarships.contains(sem)) {
            continue;
        }
        
        double calculatedScholarship = ScholarshipCalculator::calculateScholarship(grade);
        if (hasSocialScholarship) {
            calculatedScholarship += ScholarshipCalculator::SOCIAL_SCHOLARSHIP;
        }
        if (calculatedScholarship > 0.0) {
            previousSemesterScholarships[sem] = calculatedScholarship;
        }
    }
}

std::string Student::getHistoryString() const {
    if (previousSemesterGrades.empty()) {
        return "";
    }

    std::string result;
    bool first = true;
    for (const auto& [sem, grade] : previousSemesterGrades) {
        if (!first) {
            result += ";";
        }
        result += std::format("{}:{:.1f}", sem, grade);
        first = false;
    }
    return result;
}