#ifndef STUDENTHISTORYDIALOG_H
#define STUDENTHISTORYDIALOG_H

#include <memory>

#include <QString>

class QWidget;
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
};

#endif
