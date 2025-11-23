#include "ui/studentDialogBuilder.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>

#include "entities/student.h"

namespace {
QString primaryButtonStyle() {
    return QStringLiteral(
        "QPushButton { background-color: #4CAF50; color: white; padding: 8px 20px; "
        "border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #45a049; }");
}

QString dangerButtonStyle() {
    return QStringLiteral(
        "QPushButton { background-color: #f44336; color: white; padding: 8px 20px; "
        "border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #da190b; }");
}
}

StudentDialogResult::StudentDialogResult() = default;

bool StudentDialogResult::isAccepted() const { return accepted; }
void StudentDialogResult::setAccepted(bool value) { accepted = value; }

const QString& StudentDialogResult::getName() const { return name; }
void StudentDialogResult::setName(const QString& value) { name = value; }

const QString& StudentDialogResult::getSurname() const { return surname; }
void StudentDialogResult::setSurname(const QString& value) { surname = value; }

int StudentDialogResult::getSemester() const { return semester; }
void StudentDialogResult::setSemester(int value) { semester = value; }

double StudentDialogResult::getAverageGrade() const { return averageGrade; }
void StudentDialogResult::setAverageGrade(double value) { averageGrade = value; }

bool StudentDialogResult::isBudget() const { return budget; }
void StudentDialogResult::setIsBudget(bool value) { budget = value; }

int StudentDialogResult::getMissedHours() const { return missedHours; }
void StudentDialogResult::setMissedHours(int value) { missedHours = value; }

bool StudentDialogResult::hasSocialScholarship() const { return socialScholarship; }
void StudentDialogResult::setHasSocialScholarship(bool value) { socialScholarship = value; }

StudentDialogBuilder::StudentDialogBuilder(QWidget* parent) : parentWidget(parent) {}

QString StudentDialogBuilder::buildDialogStyle() const {
    return QStringLiteral(
        "QDialog { background-color: #2d2d2d; color: #ffffff; }"
        "QLabel { "
        "color: #e0e0e0; "
        "font-size: 12px; "
        "background: none; "
        "background-color: rgba(0,0,0,0); "
        "border: 0px; "
        "padding: 0px; "
        "}"
        "QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox { "
        "background-color: #1e1e1e; "
        "color: #EAEAEA; "
        "border: 2px solid #0d7377; "
        "border-radius: 4px; "
        "padding: 8px; "
        "}"
        "QLineEdit::placeholder, QSpinBox::placeholder, QDoubleSpinBox::placeholder { "
        "color: #888888; "
        "}"
        "QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus { "
        "border-color: #14a085; "
        "background-color: #252525; "
        "}"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { "
        "background-color: #1e1e1e; "
        "color: #EAEAEA; "
        "selection-background-color: #0d7377; "
        "}");
}

void StudentDialogBuilder::configureSemesterSpinBox(QSpinBox* spinBox) const {
    if (!spinBox) return;
    spinBox->setRange(1, 8);
    QObject::connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), [spinBox](int value) {
        if (value == 2) {
            spinBox->setValue(3);
        }
    });
}

StudentDialogResult StudentDialogBuilder::showAddDialog() {
    StudentDialogResult result;

    QDialog dialog(parentWidget);
    dialog.setWindowTitle("Add Student");
    dialog.setMinimumWidth(400);
    dialog.setStyleSheet(buildDialogStyle());

    QFormLayout* form = new QFormLayout(&dialog);
    form->setSpacing(15);
    form->setContentsMargins(20, 20, 20, 20);

    QLineEdit* nameField = new QLineEdit(&dialog);
    nameField->setPlaceholderText("Enter name");
    QLineEdit* surnameField = new QLineEdit(&dialog);
    surnameField->setPlaceholderText("Enter surname");

    QSpinBox* semesterField = new QSpinBox(&dialog);
    configureSemesterSpinBox(semesterField);
    semesterField->setValue(1);

    QDoubleSpinBox* avgField = new QDoubleSpinBox(&dialog);
    avgField->setRange(0.0, 10.0);
    avgField->setDecimals(2);
    avgField->setSingleStep(0.1);
    avgField->setValue(5.0);

    QComboBox* fundingField = new QComboBox(&dialog);
    fundingField->addItem("Budget");
    fundingField->addItem("Paid");

    QSpinBox* missedHoursField = new QSpinBox(&dialog);
    missedHoursField->setRange(0, 100);
    missedHoursField->setValue(0);
    missedHoursField->setToolTip("If >= 12 hours, student will lose scholarship");

    QCheckBox* socialField = new QCheckBox(&dialog);
    socialField->setText("Social Scholarship");
    socialField->setStyleSheet("color: #e0e0e0;");

    form->addRow("Name:", nameField);
    form->addRow("Surname:", surnameField);
    form->addRow("Semester:", semesterField);
    form->addRow("Average Grade:", avgField);
    form->addRow("Funding Type:", fundingField);
    form->addRow("Missed Hours:", missedHoursField);
    form->addRow("", socialField);

    QDialogButtonBox* bb =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    bb->button(QDialogButtonBox::Ok)->setText("Add");
    bb->button(QDialogButtonBox::Cancel)->setText("Cancel");
    bb->button(QDialogButtonBox::Ok)->setStyleSheet(primaryButtonStyle());
    bb->button(QDialogButtonBox::Cancel)->setStyleSheet(dangerButtonStyle());
    form->addRow(bb);

    QObject::connect(bb, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return result;
    }

    const QString name = nameField->text().trimmed();
    const QString surname = surnameField->text().trimmed();

    if (name.isEmpty() || surname.isEmpty()) {
        QMessageBox::warning(parentWidget, "Error", "Please enter both name and surname.");
        return result;
    }

    int semester = semesterField->value();
    if (semester == 2) semester = 3;

    result.setAccepted(true);
    result.setName(name);
    result.setSurname(surname);
    result.setSemester(semester);
    result.setAverageGrade(avgField->value());
    result.setIsBudget(fundingField->currentText() == "Budget");
    result.setMissedHours(missedHoursField->value());
    result.setHasSocialScholarship(socialField->isChecked());

    return result;
}

StudentDialogResult StudentDialogBuilder::showEditDialog(const std::shared_ptr<Student>& student) {
    StudentDialogResult result;
    if (!student) return result;

    QDialog dialog(parentWidget);
    dialog.setWindowTitle("Edit Student");
    dialog.setMinimumWidth(400);
    dialog.setStyleSheet(buildDialogStyle());

    QFormLayout* form = new QFormLayout(&dialog);
    form->setSpacing(15);
    form->setContentsMargins(20, 20, 20, 20);

    QLineEdit* nameField =
        new QLineEdit(QString::fromStdString(student->getName()), &dialog);
    QLineEdit* surnameField =
        new QLineEdit(QString::fromStdString(student->getSurname()), &dialog);

    QSpinBox* semesterField = new QSpinBox(&dialog);
    configureSemesterSpinBox(semesterField);
    int initialSemester = student->getSemester();
    if (initialSemester == 2) initialSemester = 3;
    semesterField->setValue(initialSemester);

    QDoubleSpinBox* avgField = new QDoubleSpinBox(&dialog);
    avgField->setRange(0.0, 10.0);
    avgField->setDecimals(2);
    avgField->setSingleStep(0.1);
    avgField->setValue(student->getAverageGrade());

    QComboBox* fundingField = new QComboBox(&dialog);
    fundingField->addItem("Budget");
    fundingField->addItem("Paid");
    fundingField->setCurrentIndex(student->getIsBudget() ? 0 : 1);

    QSpinBox* missedHoursField = new QSpinBox(&dialog);
    missedHoursField->setRange(0, 100);
    missedHoursField->setValue(student->getMissedHours());

    QCheckBox* socialField = new QCheckBox(&dialog);
    socialField->setText("Social Scholarship");
    socialField->setChecked(student->getHasSocialScholarship());
    socialField->setStyleSheet("color: #e0e0e0;");

    form->addRow("Name:", nameField);
    form->addRow("Surname:", surnameField);
    form->addRow("Semester:", semesterField);
    form->addRow("Average Grade:", avgField);
    form->addRow("Funding Type:", fundingField);
    form->addRow("Missed Hours:", missedHoursField);
    form->addRow("", socialField);

    QDialogButtonBox* bb =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    bb->button(QDialogButtonBox::Ok)->setText("Save");
    bb->button(QDialogButtonBox::Cancel)->setText("Cancel");
    bb->button(QDialogButtonBox::Ok)->setStyleSheet(primaryButtonStyle());
    bb->button(QDialogButtonBox::Cancel)->setStyleSheet(dangerButtonStyle());
    form->addRow(bb);

    QObject::connect(bb, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return result;
    }

    const QString name = nameField->text().trimmed();
    const QString surname = surnameField->text().trimmed();

    if (name.isEmpty() || surname.isEmpty()) {
        QMessageBox::warning(parentWidget, "Error", "Name and surname cannot be empty.");
        return result;
    }

    int semester = semesterField->value();
    if (semester == 2) semester = 3;

    result.setAccepted(true);
    result.setName(name);
    result.setSurname(surname);
    result.setSemester(semester);
    result.setAverageGrade(avgField->value());
    result.setIsBudget(fundingField->currentText() == "Budget");
    result.setMissedHours(missedHoursField->value());
    result.setHasSocialScholarship(socialField->isChecked());

    return result;
}
