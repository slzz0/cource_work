#include "services/historyGradeGenerator.h"

#include <cmath>

#include "entities/student.h"

HistoryGradeGenerator::HistoryGradeGenerator() = default;

double HistoryGradeGenerator::generateRandomGrade() {
    double value = distribution(generator);
    return std::round(value * 100.0) / 100.0;
}

void HistoryGradeGenerator::ensureHistoryForNewStudent(Student& student, int currentSemester) {
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
