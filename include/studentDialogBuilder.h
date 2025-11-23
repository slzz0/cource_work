#ifndef STUDENTDIALOGBUILDER_H
#define STUDENTDIALOGBUILDER_H

#include <memory>
#include <QString>

class QWidget;
class Student;

class StudentDialogResult {
public:
    StudentDialogResult();

    bool isAccepted() const;
    void setAccepted(bool accepted);

    const QString& getName() const;
    void setName(const QString& name);

    const QString& getSurname() const;
    void setSurname(const QString& surname);

    int getSemester() const;
    void setSemester(int semester);

    double getAverageGrade() const;
    void setAverageGrade(double grade);

    bool isBudget() const;
    void setIsBudget(bool budget);

    int getMissedHours() const;
    void setMissedHours(int hours);

    bool hasSocialScholarship() const;
    void setHasSocialScholarship(bool hasSocial);

private:
    bool accepted = false;
    QString name;
    QString surname;
    int semester = 1;
    double averageGrade = 5.0;
    bool budget = true;
    int missedHours = 0;
    bool socialScholarship = false;
};

class StudentDialogBuilder {
public:
    explicit StudentDialogBuilder(QWidget* parent);

    StudentDialogResult showAddDialog();
    StudentDialogResult showEditDialog(const std::shared_ptr<Student>& student);

private:
    QWidget* parentWidget;

    QString buildDialogStyle() const;
    void configureSemesterSpinBox(class QSpinBox* spinBox) const;
};

#endif