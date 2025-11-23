#include "entities/student.h"

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
    // Если студент создается как бюджетник, считаем что он был бюджетником с первого семестра
    if (isBudget) {
        budgetSemester = 1;
    }
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
    // НО: рассчитываем стипендии только за семестры, когда студент был бюджетником
    if (isBudget && budgetSemester > 0) {
        for (const auto& gradePair : previousSemesterGrades) {
            int sem = gradePair.first;
            double grade = gradePair.second;
            
            // Рассчитываем стипендию только за семестры, начиная с budgetSemester
            // (когда студент стал бюджетником)
            if (sem >= budgetSemester) {
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
    }
    
    scholarship = 0.0;

    if (!isBudget) {
        return;
    }

    // Проверяем, что текущий семестр >= budgetSemester (когда студент стал бюджетником)
    if (budgetSemester > 0 && semester < budgetSemester) {
        return;  // Студент еще не был бюджетником в этом семестре
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
    // Только если студент был бюджетником в текущем семестре
    if (isBudget && scholarship > 0.0 && semester > 0 && budgetSemester > 0 && semester >= budgetSemester) {
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
        // Только за семестры, когда студент был бюджетником (начиная с budgetSemester)
        // Это гарантирует, что стипендии за прошлые семестры не будут потеряны
        if (budgetSemester > 0) {
            for (const auto& gradePair : previousSemesterGrades) {
                int sem = gradePair.first;
                double grade = gradePair.second;
                
                // Сохраняем стипендии только за семестры, когда студент был бюджетником
                if (sem >= budgetSemester) {
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
        }
        // Сбрасываем budgetSemester при переводе на платное
        budgetSemester = -1;
    }
    // Если переводим с платного на бюджет, сохраняем текущий семестр как начало бюджетного обучения
    else if (!isBudget && newIsBudget) {
        budgetSemester = semester;
        // Удаляем стипендии за семестры до budgetSemester, так как студент был платным
        auto it = previousSemesterScholarships.begin();
        while (it != previousSemesterScholarships.end()) {
            if (it->first < budgetSemester) {
                it = previousSemesterScholarships.erase(it);
            } else {
                ++it;
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