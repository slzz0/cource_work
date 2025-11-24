#ifndef STUDENTSTATISTICSUPDATER_H
#define STUDENTSTATISTICSUPDATER_H

#include <map>
#include <memory>
#include <set>
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
    
    struct YearStats {
        int winterCount = 0;
        int summerCount = 0;
        double winterTotal = 0.0;
        double summerTotal = 0.0;
    };
    
    std::set<int> collectAllSemesters(const std::shared_ptr<Student>& student) const;
    std::vector<int> getYearSemesters(int year) const;
    bool wasStudentInYear(const std::set<int>& allSemesters, const std::vector<int>& yearSemesters) const;
    bool wasOnSummerSemester(const std::set<int>& allSemesters, const std::vector<int>& yearSemesters) const;
    double calculateWinterScholarship(const std::shared_ptr<Student>& student, int year,
                                     const std::vector<int>& yearSemesters, int currentSem) const;
    double calculateSummerScholarship(const std::shared_ptr<Student>& student, int year,
                                     const std::vector<int>& yearSemesters, int currentSem) const;
    double getScholarshipFromHistory(const std::shared_ptr<Student>& student, int sem) const;
    void populateTableRows(QTableWidget* table, const std::map<int, YearStats>& yearStats) const;
};

#endif
