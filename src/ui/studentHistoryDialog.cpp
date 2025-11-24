#include "ui/studentHistoryDialog.h"

#include <QBrush>
#include <QCheckBox>
#include <QDialog>
#include <QFont>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <algorithm>
#include <ranges>
#include <vector>

#include "services/scholarshipCalculator.h"
#include "entities/student.h"

StudentHistoryDialog::StudentHistoryDialog(QWidget* parent) : parentWidget(parent) {}

void StudentHistoryDialog::showHistory(const std::shared_ptr<Student>& student) {
    if (!student) return;

    QDialog historyDialog(parentWidget);
    historyDialog.setWindowTitle(
        QString("History: %1 %2")
            .arg(QString::fromStdString(student->getName()),
                 QString::fromStdString(student->getSurname())));
    historyDialog.setMinimumWidth(600);
    historyDialog.setMinimumHeight(500);
    historyDialog.setStyleSheet(
        "QDialog { background-color: #2d2d2d; color: #ffffff; }"
        "QLabel { "
        "color: #e0e0e0; "
        "font-size: 12px; "
        "background: none; "
        "background-color: rgba(0,0,0,0); "
        "border: 0px; "
        "padding: 0px; "
        "}");

    auto layout = new QVBoxLayout(&historyDialog);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);

    auto currentInfoLabel = new QLabel("Current Semester Information:", &historyDialog);
    currentInfoLabel->setStyleSheet(
        "font-weight: bold; font-size: 14px; color: #14a085; background: none; background-color: "
        "rgba(0,0,0,0); border: 0px; padding: 0px;");
    layout->addWidget(currentInfoLabel);

    auto currentInfoTable = setupCurrentInfoTable(&historyDialog, layout);
    populateCurrentInfoTable(currentInfoTable, student);

    auto prevGradesLabel = new QLabel("Previous Semesters Performance:", &historyDialog);
    prevGradesLabel->setStyleSheet(
        "font-weight: bold; font-size: 14px; color: #14a085; background: none; background-color: "
        "rgba(0,0,0,0); border: 0px; padding: 0px;");
    layout->addWidget(prevGradesLabel);

    auto historyTable = setupHistoryTable(&historyDialog);
    int admissionYear = getAdmissionYearFromSemester(student->getSemester());
    populateHistoryTable(historyTable, student, admissionYear);

    layout->addWidget(historyTable, 1);

    auto closeButton = new QPushButton("Close", &historyDialog);
    closeButton->setStyleSheet(
        "QPushButton { "
        "background-color: #0d7377; "
        "color: white; "
        "padding: 10px 30px; "
        "border-radius: 5px; "
        "font-weight: bold; "
        "font-size: 12px; "
        "}"
        "QPushButton:hover { background-color: #14a085; }");
    QObject::connect(closeButton, &QPushButton::clicked, &historyDialog, &QDialog::accept);
    auto buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    historyDialog.exec();
}

QTableWidget* StudentHistoryDialog::setupCurrentInfoTable(QDialog* dialog, QVBoxLayout* layout) const {
    auto table = new QTableWidget(dialog);
    table->setColumnCount(2);
    table->setRowCount(8);
    table->setHorizontalHeaderLabels(QStringList() << "Property" << "Value");
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->setStyleSheet(
        "QTableWidget {"
        "gridline-color: #2d2d2d;"
        "background-color: #1B1B1B;"
        "alternate-background-color: #1E1E1E;"
        "border: 2px solid #0d7377;"
        "border-radius: 8px;"
        "}"
        "QTableWidget::item {"
        "padding: 12px;"
        "border: none;"
        "color: #EAEAEA;"
        "font-size: 11px;"
        "}"
        "QTableWidget::item:selected {"
        "background-color: #0d7377;"
        "}"
        "QTableWidget::item:selected:alternate {"
        "background-color: #0d7377;"
        "}"
        "QHeaderView::section {"
        "background-color: #2A2A2A;"
        "color: #EAEAEA;"
        "padding: 14px;"
        "border: none;"
        "border-bottom: 2px solid #0d7377;"
        "font-size: 12px;"
        "font-weight: bold;"
        "}");
    layout->addWidget(table);
    return table;
}

void StudentHistoryDialog::populateCurrentInfoTable(QTableWidget* table, const std::shared_ptr<Student>& student) const {
    const QStringList properties = {"Name",          "Surname",     "Course",       "Semester",
                                    "Average Grade", "Funding",     "Missed Hours", "Social"};
    const QStringList values = {
        QString::fromStdString(student->getName()),
        QString::fromStdString(student->getSurname()),
        QString::number(student->getCourse()),
        QString::number(student->getSemester()),
        QString::number(student->getAverageGrade(), 'f', 2),
        student->getIsBudget() ? "Budget" : "Paid",
        QString::number(student->getMissedHours()),
        student->getHasSocialScholarship() ? "Yes" : "No"};

    for (int row = 0; row < properties.size(); ++row) {
        auto propertyItem = new QTableWidgetItem(properties[row]);
        propertyItem->setForeground(QBrush(QColor(234, 234, 234)));
        propertyItem->setFlags(propertyItem->flags() & ~Qt::ItemIsSelectable);
        table->setItem(row, 0, propertyItem);

        auto valueItem = new QTableWidgetItem(values[row]);
        valueItem->setForeground(QBrush(QColor(234, 234, 234)));
        valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsSelectable);
        table->setItem(row, 1, valueItem);
    }

    table->resizeColumnsToContents();
    table->horizontalHeader()->setStretchLastSection(true);
}

QTableWidget* StudentHistoryDialog::setupHistoryTable(QDialog* dialog) const {
    auto table = new QTableWidget(dialog);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels(QStringList() << "Semester"
                                                   << "Average Grade"
                                                   << "Scholarship (BYN)");
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->setStyleSheet(
        "QTableWidget {"
        "gridline-color: #2d2d2d;"
        "background-color: #1B1B1B;"
        "alternate-background-color: #1E1E1E;"
        "border: 2px solid #0d7377;"
        "border-radius: 8px;"
        "}"
        "QTableWidget::item {"
        "padding: 12px;"
        "border: none;"
        "color: #EAEAEA;"
        "font-size: 11px;"
        "}"
        "QTableWidget::item:selected {"
        "background-color: #0d7377;"
        "}"
        "QTableWidget::item:selected:alternate {"
        "background-color: #0d7377;"
        "}"
        "QHeaderView::section {"
        "background-color: #2A2A2A;"
        "color: #EAEAEA;"
        "padding: 14px;"
        "border: none;"
        "border-bottom: 2px solid #0d7377;"
        "font-size: 12px;"
        "font-weight: bold;"
        "}");
    return table;
}

void StudentHistoryDialog::populateHistoryTable(QTableWidget* table, const std::shared_ptr<Student>& student, int admissionYear) const {
    const auto& previousGrades = student->getPreviousSemesterGrades();

    if (previousGrades.empty()) {
        table->setRowCount(1);
        auto noDataText = "No previous semester data available. This student is on their first semester.";
        auto noDataItem = new QTableWidgetItem(noDataText);
        noDataItem->setForeground(QBrush(QColor(234, 234, 234)));
        noDataItem->setFlags(noDataItem->flags() & ~Qt::ItemIsSelectable);
        noDataItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(0, 0, noDataItem);
        for (int col = 1; col < 3; ++col) {
            auto emptyItem = new QTableWidgetItem("");
            emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsSelectable);
            table->setItem(0, col, emptyItem);
        }
        table->setSpan(0, 0, 1, 3);
        table->horizontalHeader()->setStretchLastSection(true);
        return;
    }

    std::vector<std::pair<int, double>> sortedHistory(previousGrades.begin(), previousGrades.end());
    std::ranges::sort(sortedHistory);
    table->setRowCount(static_cast<int>(sortedHistory.size()));

    for (int i = 0; i < static_cast<int>(sortedHistory.size()); ++i) {
        int sem = sortedHistory[i].first;
        double grade = sortedHistory[i].second;
        double scholarship = calculateScholarshipForSemester(student, sem, grade);
        createHistoryRow(table, i, sem, grade, scholarship, admissionYear);
    }

    table->resizeColumnsToContents();
    table->horizontalHeader()->setStretchLastSection(true);
}

double StudentHistoryDialog::calculateScholarshipForSemester(const std::shared_ptr<Student>& student, int sem, double grade) const {
    int budgetSem = student->getBudgetSemester();
    if (budgetSem <= 0 || sem < budgetSem) {
        return 0.0;
    }

    const auto& scholarshipHistory = student->getPreviousSemesterScholarships();
    if (auto scholarshipIt = scholarshipHistory.find(sem); scholarshipIt != scholarshipHistory.end()) {
        return scholarshipIt->second;
    }

    if (student->getIsBudget()) {
        return ScholarshipCalculator::calculateScholarship(grade);
    }

    return 0.0;
}

void StudentHistoryDialog::createHistoryRow(QTableWidget* table, int row, int sem, double grade, double scholarship, int admissionYear) const {
    auto year = getYearForSemester(sem, admissionYear);
    auto sessionType = getSessionTypeForSemester(sem);
    auto semesterText = QString("%1 (%2 %3)").arg(sem).arg(year).arg(sessionType);
    auto semesterItem = new QTableWidgetItem(semesterText);
    semesterItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    semesterItem->setForeground(QBrush(QColor(234, 234, 234)));
    semesterItem->setFlags(semesterItem->flags() & ~Qt::ItemIsSelectable);
    table->setItem(row, 0, semesterItem);

    auto gradeItem = new QTableWidgetItem(QString::number(grade, 'f', 2));
    gradeItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    gradeItem->setForeground(QBrush(QColor(234, 234, 234)));
    gradeItem->setFlags(gradeItem->flags() & ~Qt::ItemIsSelectable);
    table->setItem(row, 1, gradeItem);

    auto scholarshipItem = new QTableWidgetItem(QString::number(scholarship, 'f', 2));
    scholarshipItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    if (scholarship > 0) {
        scholarshipItem->setForeground(QBrush(QColor(76, 175, 80)));
        scholarshipItem->setFont(QFont("", -1, QFont::Bold));
    } else {
        scholarshipItem->setForeground(QBrush(QColor(158, 158, 158)));
    }
    scholarshipItem->setFlags(scholarshipItem->flags() & ~Qt::ItemIsSelectable);
    table->setItem(row, 2, scholarshipItem);
}

int StudentHistoryDialog::getAdmissionYearFromSemester(int semester) const {
    return 2026 - ((semester + 1) / 2);
}

int StudentHistoryDialog::getYearForSemester(int semester, int admissionYear) const {
    if (semester % 2 == 1) {
        return admissionYear + (semester - 1) / 2;
    }
    return admissionYear + semester / 2;
}

QString StudentHistoryDialog::getSessionTypeForSemester(int semester) const {
    return (semester % 2 == 1) ? "Winter" : "Summer";
}
