#ifndef HISTORYGRADEGENERATOR_H
#define HISTORYGRADEGENERATOR_H

#include <memory>
#include <random>
#include <vector>

class Student;

class HistoryGradeGenerator {
public:
    HistoryGradeGenerator();

    void ensureHistoryForNewStudent(Student& student, int currentSemester);
    void handleSemesterChange(Student& student, int oldSemester, double oldAverageGrade,
                              int newSemester);
    void fillMissingHistoryForAll(const std::vector<std::shared_ptr<Student>>& students);

private:
    double generateRandomGrade() const;
};

#endif