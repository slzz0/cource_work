#ifndef STUDENTDIALOGBUILDER_H
#define STUDENTDIALOGBUILDER_H

#include <memory>
#include <QString>

class QWidget;
class QDialog;
class QFormLayout;
class QDialogButtonBox;
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

struct DialogFields {
    class QLineEdit* nameField = nullptr;
    class QLineEdit* surnameField = nullptr;
    class QSpinBox* semesterField = nullptr;
    class QDoubleSpinBox* avgField = nullptr;
    class QComboBox* fundingField = nullptr;
    class QSpinBox* missedHoursField = nullptr;
    class QCheckBox* socialField = nullptr;
};

class StudentDialogBuilder {
public:
    explicit StudentDialogBuilder(QWidget* parent);

    StudentDialogResult showAddDialog() const;
    StudentDialogResult showEditDialog(const std::shared_ptr<Student>& student) const;

private:
    StudentDialogResult showDialog(const QString& title, const QString& okButtonText,
                                   const QString& errorMessage,
                                   const std::shared_ptr<Student>& student) const;

    QWidget* parentWidget;

    QString buildDialogStyle() const;
    void configureSemesterSpinBox(class QSpinBox* spinBox) const;
    void setupDialog(QDialog* dialog, const QString& title) const;
    QFormLayout* createFormLayout(QDialog* dialog) const;
    DialogFields createFields(QDialog* dialog, const std::shared_ptr<Student>& student = nullptr) const;
    void addFieldsToForm(QFormLayout* form, const DialogFields& fields) const;
    QDialogButtonBox* createButtonBox(QDialog* dialog, const QString& okText) const;
    void connectButtonBox(const QDialogButtonBox* bb, const QDialog* dialog) const;
    bool validateAndFillResult(const DialogFields& fields, StudentDialogResult& result, const QString& errorMessage) const;
};

#endif