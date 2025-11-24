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
    // Если студент создается как бюджетник, считаем что он был бюджетником с первого семестра
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
    // Сохраняем стипендии за все предыдущие семестры перед пересчетом
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
    // Сохраняем стипендии за все предыдущие семестры перед пересчетом
    // Это гарантирует, что при переводе на платное стипендии не будут потеряны
    // НО: рассчитываем стипендии только за семестры, когда студент был бюджетником
    if (!isBudget || budgetSemester <= 0) {
        return;
    }
    
    saveScholarshipsForBudgetSemesters(budgetSemester);
}

bool Student::isEligibleForScholarship() const {
    // Если студент сейчас платный, он не может получать стипендию
    if (!isBudget) {
        return false;
    }
    // Проверяем, что текущий семестр >= budgetSemester (когда студент стал бюджетником)
    if (budgetSemester > 0 && semester < budgetSemester) {
        return false;  // Студент еще не был бюджетником в этом семестре
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
    // Если тип финансирования не изменился, ничего не делаем
    if (isBudget == newIsBudget) {
        return;
    }
    
    bool wasBudget = isBudget;
    isBudget = newIsBudget;
    
    // Если переводим с бюджета на платное
    if (wasBudget && !newIsBudget) {
        // Обнуляем стипендию только для текущего семестра
        scholarship = 0.0;
        // Удаляем стипендию из истории только для текущего семестра
        // За прошлые семестры ничего не меняется - стипендии остаются в истории
        previousSemesterScholarships.erase(semester);
        // НЕ сбрасываем budgetSemester - оставляем его для сохранения истории стипендий за прошлые семестры
        // budgetSemester будет использоваться для проверки, был ли студент бюджетником в прошлых семестрах
    }
    // Если переводим с платного на бюджет
    else if (!wasBudget && newIsBudget) {
        // Устанавливаем текущий семестр как начало бюджетного обучения
        budgetSemester = semester;
        // Удаляем стипендии за семестры до budgetSemester (когда студент был платным)
        std::erase_if(previousSemesterScholarships,
                      [this](const auto& pair) { return pair.first < budgetSemester; });
        // Рассчитываем стипендию только для текущего семестра
        if (isEligibleForScholarship()) {
            calculateCurrentScholarship();
            saveCurrentScholarshipToHistory();
        } else {
            scholarship = 0.0;
        }
        // За прошлые семестры ничего не меняется
    }
}

void Student::saveScholarshipsForBudgetSemesters(int startSemester) {
    for (const auto& [sem, grade] : previousSemesterGrades) {
        // Сохраняем стипендии только за семестры, когда студент был бюджетником
        if (sem < startSemester) {
            continue;
        }
        
        // Пропускаем текущий семестр - он обрабатывается отдельно
        if (sem == semester) {
            continue;
        }
        
        // Если стипендия для этого семестра еще не сохранена, вычисляем и сохраняем
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