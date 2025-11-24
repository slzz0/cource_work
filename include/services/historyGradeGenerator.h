#ifndef HISTORYGRADEGENERATOR_H
#define HISTORYGRADEGENERATOR_H

#include <memory>
#include <random>
#include <vector>

class Student;

class HistoryGradeGenerator {
public:
    HistoryGradeGenerator();

    void ensureHistoryForNewStudent(Student& student, int currentSemester) const;
    void handleSemesterChange(Student& student, int oldSemester, double oldAverageGrade,
                              int newSemester) const;
    void fillMissingHistoryForAll(const std::vector<std::shared_ptr<Student>>& students) const;

private:
    double generateRandomGrade() const;
};

#endif