#ifndef STUDENTSTATISTICSUPDATER_H
#define STUDENTSTATISTICSUPDATER_H

#include <map>
#include <memory>
#include <vector>

class QLabel;
class QTableWidget;
class Student;

class StudentStatisticsUpdater {
public:
    void updateGeneralStatistics(const std::vector<std::shared_ptr<Student>>& students,
                                 QLabel* totalLabel,
                                 QLabel* budgetLabel,
                                 QLabel* paidLabel,
                                 QLabel* totalScholarshipLabel) const;

    void updateSemesterTable(const std::vector<std::shared_ptr<Student>>& students,
                             QTableWidget* table) const;

private:
    int getYearForSemester(int semester) const;
    int getYearForSemester(int semester, int admissionYear) const;
};

#endif
