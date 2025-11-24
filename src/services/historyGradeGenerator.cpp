#include "services/historyGradeGenerator.h"

#include <cmath>
#include <random>

#include "entities/student.h"

HistoryGradeGenerator::HistoryGradeGenerator() = default;

double HistoryGradeGenerator::generateRandomGrade() const {
    // Используем std::random_device для генерации случайных чисел
    thread_local static std::random_device rd;
    thread_local static std::uniform_real_distribution distribution{5.0, 10.0};
    
    double value = distribution(rd);
    return std::round(value * 100.0) / 100.0;
}

void HistoryGradeGenerator::ensureHistoryForNewStudent(Student& student, int currentSemester) const {
    if (currentSemester <= 1) return;
    const auto& existingHistory = student.getPreviousSemesterGrades();
    for (int sem = 1; sem < currentSemester; ++sem) {
        if (!existingHistory.contains(sem)) {
            student.addPreviousGrade(sem, generateRandomGrade());
        }
    }
}

void HistoryGradeGenerator::handleSemesterChange(Student& student, int oldSemester,
                                                 double oldAverageGrade, int newSemester) {
    if (oldSemester != newSemester && oldSemester > 0) {
        student.addPreviousGrade(oldSemester, oldAverageGrade);
    }
    ensureHistoryForNewStudent(student, newSemester);
}

void HistoryGradeGenerator::fillMissingHistoryForAll(
    const std::vector<std::shared_ptr<Student>>& students) {
    for (const auto& student : students) {
        if (!student) continue;
        ensureHistoryForNewStudent(*student, student->getSemester());
    }
}
