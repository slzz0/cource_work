#include "studentHistoryDialog.h"

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
#include <vector>

#include "scholarshipCalculator.h"
#include "student.h"

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

    QVBoxLayout* layout = new QVBoxLayout(&historyDialog);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);

    QLabel* currentInfoLabel = new QLabel("Current Semester Information:", &historyDialog);
    currentInfoLabel->setStyleSheet(
        "font-weight: bold; font-size: 14px; color: #14a085; background: none; background-color: "
        "rgba(0,0,0,0); border: 0px; padding: 0px;");
    layout->addWidget(currentInfoLabel);

    QTableWidget* currentInfoTable = new QTableWidget(&historyDialog);
    currentInfoTable->setColumnCount(2);
    currentInfoTable->setRowCount(8);
    currentInfoTable->setHorizontalHeaderLabels(QStringList() << "Property" << "Value");
    currentInfoTable->verticalHeader()->setVisible(false);
    currentInfoTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    currentInfoTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    currentInfoTable->setAlternatingRowColors(true);
    currentInfoTable->setStyleSheet(
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
    layout->addWidget(currentInfoTable);

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
        QTableWidgetItem* propertyItem = new QTableWidgetItem(properties[row]);
        propertyItem->setForeground(QBrush(QColor(234, 234, 234)));
        propertyItem->setFlags(propertyItem->flags() & ~Qt::ItemIsSelectable);
        currentInfoTable->setItem(row, 0, propertyItem);

        QTableWidgetItem* valueItem = new QTableWidgetItem(values[row]);
        valueItem->setForeground(QBrush(QColor(234, 234, 234)));
        valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsSelectable);
        currentInfoTable->setItem(row, 1, valueItem);
    }

    currentInfoTable->resizeColumnsToContents();
    currentInfoTable->horizontalHeader()->setStretchLastSection(true);

    QLabel* prevGradesLabel = new QLabel("Previous Semesters Performance:", &historyDialog);
    prevGradesLabel->setStyleSheet(
        "font-weight: bold; font-size: 14px; color: #14a085; background: none; background-color: "
        "rgba(0,0,0,0); border: 0px; padding: 0px;");
    layout->addWidget(prevGradesLabel);

    QTableWidget* historyTable = new QTableWidget(&historyDialog);
    historyTable->setColumnCount(3);
    historyTable->setHorizontalHeaderLabels(QStringList() << "Semester"
                                                          << "Average Grade"
                                                          << "Scholarship (BYN)");
    historyTable->verticalHeader()->setVisible(false);
    historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    historyTable->setAlternatingRowColors(true);
    historyTable->setStyleSheet(
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

    const auto& previousGrades = student->getPreviousSemesterGrades();
    int admissionYear = getAdmissionYearFromSemester(student->getSemester());

    if (previousGrades.empty()) {
        historyTable->setRowCount(1);
        QString noDataText =
            "No previous semester data available. This student is on their first semester.";
        QTableWidgetItem* noDataItem = new QTableWidgetItem(noDataText);
        noDataItem->setForeground(QBrush(QColor(234, 234, 234)));
        noDataItem->setFlags(noDataItem->flags() & ~Qt::ItemIsSelectable);
        noDataItem->setTextAlignment(Qt::AlignCenter);
        historyTable->setItem(0, 0, noDataItem);
        for (int col = 1; col < 3; ++col) {
            QTableWidgetItem* emptyItem = new QTableWidgetItem("");
            emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsSelectable);
            historyTable->setItem(0, col, emptyItem);
        }
        historyTable->setSpan(0, 0, 1, 3);
        historyTable->horizontalHeader()->setStretchLastSection(true);
    } else {
        std::vector<std::pair<int, double>> sortedHistory(previousGrades.begin(),
                                                          previousGrades.end());
        std::sort(sortedHistory.begin(), sortedHistory.end());

        historyTable->setRowCount(static_cast<int>(sortedHistory.size()));

        for (int i = 0; i < static_cast<int>(sortedHistory.size()); ++i) {
            int sem = sortedHistory[i].first;
            double grade = sortedHistory[i].second;

            // Используем сохраненную историю стипендий, если она есть
            // НО: показываем стипендии только за семестры, когда студент был бюджетником
            double prevScholarship = 0.0;
            int budgetSem = student->getBudgetSemester();
            
            // Показываем стипендию только если семестр >= budgetSemester (когда студент стал бюджетником)
            if (budgetSem > 0 && sem >= budgetSem) {
                const auto& scholarshipHistory = student->getPreviousSemesterScholarships();
                auto scholarshipIt = scholarshipHistory.find(sem);
                if (scholarshipIt != scholarshipHistory.end()) {
                    // Используем сохраненную стипендию из истории
                    prevScholarship = scholarshipIt->second;
                } else {
                    // Если истории нет, вычисляем из оценки только если студент сейчас на бюджете
                    // и семестр >= budgetSemester
                    if (student->getIsBudget()) {
                        prevScholarship = ScholarshipCalculator::calculateScholarship(grade);
                    }
                }
            }
            // Если семестр < budgetSemester, студент был платным в этом семестре, стипендия = 0

            int year = getYearForSemester(sem, admissionYear);
            QString sessionType = getSessionTypeForSemester(sem);
            QString semesterText = QString("%1 (%2 %3)").arg(sem).arg(year).arg(sessionType);
            QTableWidgetItem* semesterItem = new QTableWidgetItem(semesterText);
            semesterItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            semesterItem->setForeground(QBrush(QColor(234, 234, 234)));
            semesterItem->setFlags(semesterItem->flags() & ~Qt::ItemIsSelectable);
            historyTable->setItem(i, 0, semesterItem);

            QTableWidgetItem* gradeItem = new QTableWidgetItem(QString::number(grade, 'f', 2));
            gradeItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            gradeItem->setForeground(QBrush(QColor(234, 234, 234)));
            gradeItem->setFlags(gradeItem->flags() & ~Qt::ItemIsSelectable);
            historyTable->setItem(i, 1, gradeItem);

            QTableWidgetItem* scholarshipItem =
                new QTableWidgetItem(QString::number(prevScholarship, 'f', 2));
            scholarshipItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            if (prevScholarship > 0) {
                scholarshipItem->setForeground(QBrush(QColor(76, 175, 80)));
                scholarshipItem->setFont(QFont("", -1, QFont::Bold));
            } else {
                scholarshipItem->setForeground(QBrush(QColor(158, 158, 158)));
            }
            scholarshipItem->setFlags(scholarshipItem->flags() & ~Qt::ItemIsSelectable);
            historyTable->setItem(i, 2, scholarshipItem);
        }

        historyTable->resizeColumnsToContents();
        historyTable->horizontalHeader()->setStretchLastSection(true);
    }

    layout->addWidget(historyTable, 1);

    QPushButton* closeButton = new QPushButton("Close", &historyDialog);
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
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    historyDialog.exec();
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
