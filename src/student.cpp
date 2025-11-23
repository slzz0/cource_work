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
    // Сохраняем стипендии за все предыдущие семестры перед пересчетом
    // Это гарантирует, что при переводе на платное стипендии не будут потеряны
    if (isBudget) {
        for (const auto& gradePair : previousSemesterGrades) {
            int sem = gradePair.first;
            double grade = gradePair.second;
            
            // Если стипендия для этого семестра еще не сохранена, вычисляем и сохраняем
            if (previousSemesterScholarships.find(sem) == previousSemesterScholarships.end()) {
                double calculatedScholarship = ScholarshipCalculator::calculateScholarship(grade);
                if (hasSocialScholarship) {
                    calculatedScholarship += ScholarshipCalculator::SOCIAL_SCHOLARSHIP;
                }
                if (calculatedScholarship > 0.0) {
                    previousSemesterScholarships[sem] = calculatedScholarship;
                }
            }
        }
    }
    
    scholarship = 0.0;

    if (!isBudget) {
        return;
    }

    if (missedHours >= 12) {
        if (hasSocialScholarship) {
            scholarship = ScholarshipCalculator::SOCIAL_SCHOLARSHIP;
        }
        // Сохраняем стипендию в историю для текущего семестра после пересчета
        if (scholarship > 0.0) {
            previousSemesterScholarships[semester] = scholarship;
        }
        return;
    }

    scholarship = ScholarshipCalculator::calculateScholarship(averageGrade);

    if (hasSocialScholarship) {
        scholarship += ScholarshipCalculator::SOCIAL_SCHOLARSHIP;
    }
    
    // Сохраняем стипендию в историю для текущего семестра после пересчета
    if (scholarship > 0.0) {
        previousSemesterScholarships[semester] = scholarship;
    }
}

void Student::setSemester(int newSemester) {
    // Перед сменой семестра сохраняем текущую стипендию в историю
    if (isBudget && scholarship > 0.0 && semester > 0) {
        previousSemesterScholarships[semester] = scholarship;
    }
    this->semester = newSemester;
    this->course = (semester - 1) / 2 + 1;
}

void Student::setIsBudget(bool newIsBudget) {
    // Если переводим с бюджета на платное, сохраняем стипендии за ВСЕ предыдущие семестры
    if (isBudget && !newIsBudget) {
        // Сохраняем текущую стипендию для текущего семестра
        if (scholarship > 0.0) {
            previousSemesterScholarships[semester] = scholarship;
        }
        
        // Сохраняем стипендии за все предыдущие семестры из истории оценок
        // Это гарантирует, что стипендии за прошлые семестры не будут потеряны
        for (const auto& gradePair : previousSemesterGrades) {
            int sem = gradePair.first;
            double grade = gradePair.second;
            
            // Если стипендия для этого семестра еще не сохранена, вычисляем и сохраняем
            if (previousSemesterScholarships.find(sem) == previousSemesterScholarships.end()) {
                double calculatedScholarship = ScholarshipCalculator::calculateScholarship(grade);
                if (hasSocialScholarship) {
                    calculatedScholarship += ScholarshipCalculator::SOCIAL_SCHOLARSHIP;
                }
                if (calculatedScholarship > 0.0) {
                    previousSemesterScholarships[sem] = calculatedScholarship;
                }
            }
        }
    }
    isBudget = newIsBudget;
    // При переводе на платное стипендия обнуляется только для текущего семестра
    // История стипендий за предыдущие семестры сохраняется
    if (!newIsBudget) {
        scholarship = 0.0;
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