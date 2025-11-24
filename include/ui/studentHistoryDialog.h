#ifndef STUDENTHISTORYDIALOG_H
#define STUDENTHISTORYDIALOG_H

#include <memory>

#include <QString>

class QWidget;
class QDialog;
class QTableWidget;
class QVBoxLayout;
class Student;

class StudentHistoryDialog {
public:
    explicit StudentHistoryDialog(QWidget* parent);

    void showHistory(const std::shared_ptr<Student>& student);

private:
    QWidget* parentWidget = nullptr;

    int getAdmissionYearFromSemester(int semester) const;
    int getYearForSemester(int semester, int admissionYear) const;
    QString getSessionTypeForSemester(int semester) const;
    
    QTableWidget* setupCurrentInfoTable(QDialog* dialog, QVBoxLayout* layout) const;
    void populateCurrentInfoTable(QTableWidget* table, const std::shared_ptr<Student>& student) const;
    QTableWidget* setupHistoryTable(QDialog* dialog) const;
    void populateHistoryTable(QTableWidget* table, const std::shared_ptr<Student>& student, int admissionYear) const;
    double calculateScholarshipForSemester(const std::shared_ptr<Student>& student, int sem, double grade) const;
    void createHistoryRow(QTableWidget* table, int row, int sem, double grade, double scholarship, int admissionYear) const;
};

#endif
