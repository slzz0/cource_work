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

int normalizeSemester(int semester) {
    return (semester == 2) ? 3 : semester;
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
        int normalized = normalizeSemester(value);
        if (normalized != value) {
            spinBox->setValue(normalized);
        }
    });
}

void StudentDialogBuilder::setupDialog(QDialog* dialog, const QString& title) const {
    dialog->setWindowTitle(title);
    dialog->setMinimumWidth(400);
    dialog->setStyleSheet(buildDialogStyle());
}

QFormLayout* StudentDialogBuilder::createFormLayout(QDialog* dialog) const {
    auto form = new QFormLayout(dialog);
    form->setSpacing(15);
    form->setContentsMargins(20, 20, 20, 20);
    return form;
}

DialogFields StudentDialogBuilder::createFields(QDialog* dialog, const std::shared_ptr<Student>& student) const {
    DialogFields fields;
    
    if (student) {
        fields.nameField = new QLineEdit(QString::fromStdString(student->getName()), dialog);
        fields.surnameField = new QLineEdit(QString::fromStdString(student->getSurname()), dialog);
    } else {
        fields.nameField = new QLineEdit(dialog);
        fields.nameField->setPlaceholderText("Enter name");
        fields.surnameField = new QLineEdit(dialog);
        fields.surnameField->setPlaceholderText("Enter surname");
    }
    
    fields.semesterField = new QSpinBox(dialog);
    configureSemesterSpinBox(fields.semesterField);
    if (student) {
        fields.semesterField->setValue(normalizeSemester(student->getSemester()));
    } else {
        fields.semesterField->setValue(1);
    }
    
    fields.avgField = new QDoubleSpinBox(dialog);
    fields.avgField->setRange(0.0, 10.0);
    fields.avgField->setDecimals(2);
    fields.avgField->setSingleStep(0.1);
    fields.avgField->setValue(student ? student->getAverageGrade() : 5.0);
    
    fields.fundingField = new QComboBox(dialog);
    fields.fundingField->addItem("Budget");
    fields.fundingField->addItem("Paid");
    if (student) {
        fields.fundingField->setCurrentIndex(student->getIsBudget() ? 0 : 1);
    }
    
    fields.missedHoursField = new QSpinBox(dialog);
    fields.missedHoursField->setRange(0, 100);
    fields.missedHoursField->setValue(student ? student->getMissedHours() : 0);
    if (!student) {
        fields.missedHoursField->setToolTip("If >= 12 hours, student will lose scholarship");
    }
    
    fields.socialField = new QCheckBox(dialog);
    fields.socialField->setText("Social Scholarship");
    fields.socialField->setStyleSheet("color: #e0e0e0;");
    if (student) {
        fields.socialField->setChecked(student->getHasSocialScholarship());
    }
    
    return fields;
}

void StudentDialogBuilder::addFieldsToForm(QFormLayout* form, const DialogFields& fields) const {
    form->addRow("Name:", fields.nameField);
    form->addRow("Surname:", fields.surnameField);
    form->addRow("Semester:", fields.semesterField);
    form->addRow("Average Grade:", fields.avgField);
    form->addRow("Funding Type:", fields.fundingField);
    form->addRow("Missed Hours:", fields.missedHoursField);
    form->addRow("", fields.socialField);
}

QDialogButtonBox* StudentDialogBuilder::createButtonBox(QDialog* dialog, const QString& okText) const {
    auto bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
    bb->button(QDialogButtonBox::Ok)->setText(okText);
    bb->button(QDialogButtonBox::Cancel)->setText("Cancel");
    bb->button(QDialogButtonBox::Ok)->setStyleSheet(primaryButtonStyle());
    bb->button(QDialogButtonBox::Cancel)->setStyleSheet(dangerButtonStyle());
    return bb;
}

void StudentDialogBuilder::connectButtonBox(QDialogButtonBox* bb, QDialog* dialog) const {
    QObject::connect(bb, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
}

bool StudentDialogBuilder::validateAndFillResult(const DialogFields& fields, StudentDialogResult& result, const QString& errorMessage) const {
    const QString name = fields.nameField->text().trimmed();
    const QString surname = fields.surnameField->text().trimmed();
    
    if (name.isEmpty() || surname.isEmpty()) {
        QMessageBox::warning(parentWidget, "Error", errorMessage);
        return false;
    }
    
    result.setAccepted(true);
    result.setName(name);
    result.setSurname(surname);
    result.setSemester(normalizeSemester(fields.semesterField->value()));
    result.setAverageGrade(fields.avgField->value());
    result.setIsBudget(fields.fundingField->currentText() == "Budget");
    result.setMissedHours(fields.missedHoursField->value());
    result.setHasSocialScholarship(fields.socialField->isChecked());
    
    return true;
}

StudentDialogResult StudentDialogBuilder::showDialog(const QString& title, const QString& okButtonText,
                                                     const QString& errorMessage,
                                                     const std::shared_ptr<Student>& student) const {
    StudentDialogResult result;
    
    QDialog dialog(parentWidget);
    setupDialog(&dialog, title);
    
    auto form = createFormLayout(&dialog);
    auto fields = createFields(&dialog, student);
    addFieldsToForm(form, fields);
    
    auto bb = createButtonBox(&dialog, okButtonText);
    form->addRow(bb);
    connectButtonBox(bb, &dialog);
    
    if (dialog.exec() != QDialog::Accepted) {
        return result;
    }
    
    validateAndFillResult(fields, result, errorMessage);
    
    return result;
}

StudentDialogResult StudentDialogBuilder::showAddDialog() {
    return showDialog("Add Student", "Add", "Please enter both name and surname.", nullptr);
}

StudentDialogResult StudentDialogBuilder::showEditDialog(const std::shared_ptr<Student>& student) {
    if (!student) {
        return StudentDialogResult();
    }
    return showDialog("Edit Student", "Save", "Name and surname cannot be empty.", student);
}
