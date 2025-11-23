#include "services/studentStatisticsUpdater.h"

#include <QBrush>
#include <QFont>
#include <QLabel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <set>

#include "services/scholarshipCalculator.h"
#include "entities/student.h"

void StudentStatisticsUpdater::updateGeneralStatistics(
    const std::vector<std::shared_ptr<Student>>& students,
    QLabel* totalLabel,
    QLabel* budgetLabel,
    QLabel* paidLabel,
    QLabel* totalScholarshipLabel) const {
    if (!totalLabel || !budgetLabel || !paidLabel || !totalScholarshipLabel) return;

    size_t total = students.size();
    size_t budget = 0;
    size_t paid = 0;
    double totalScholarship = 0.0;

    for (const auto& student : students) {
        if (!student) continue;
        if (student->getIsBudget()) {
            budget++;
        } else {
            paid++;
        }
        totalScholarship += student->getScholarship();
    }

    totalLabel->setText(QString("Total Students: %1").arg(total));
    budgetLabel->setText(QString("Budget Students: %1").arg(budget));
    paidLabel->setText(QString("Paid Students: %1").arg(paid));
    totalScholarshipLabel->setText(
        QString("Total Scholarships: %1 BYN").arg(totalScholarship, 0, 'f', 2));
}

void StudentStatisticsUpdater::updateSemesterTable(
    const std::vector<std::shared_ptr<Student>>& students, QTableWidget* table) const {
    if (!table) return;

    table->setRowCount(0);

    struct YearStats {
        int winterCount = 0;
        int summerCount = 0;
        double winterTotal = 0.0;
        double summerTotal = 0.0;
    };

    std::map<int, YearStats> yearStats;
    for (int year = 2022; year <= 2025; ++year) {
        yearStats[year] = YearStats{};
    }

    for (const auto& student : students) {
        if (!student) continue;
        bool isBudget = student->getIsBudget();
        int currentSem = student->getSemester();
        const auto& history = student->getPreviousSemesterGrades();

        std::set<int> allSemesters;
        allSemesters.insert(currentSem);
        for (const auto& entry : history) {
            allSemesters.insert(entry.first);
        }

        for (int year = 2022; year <= 2025; ++year) {
            std::vector<int> yearSemesters;
            for (int sem = 1; sem <= 8; ++sem) {
                if (getYearForSemester(sem) == year) {
                    yearSemesters.push_back(sem);
                }
            }

            bool wasInYear = false;
            for (int sem : yearSemesters) {
                if (allSemesters.find(sem) != allSemesters.end()) {
                    wasInYear = true;
                    break;
                }
            }

            if (!wasInYear) continue;

            auto& stats = yearStats[year];
            stats.winterCount += 1;

            double winterScholarship = 0.0;
            if (isBudget) {
                int budgetSem = student->getBudgetSemester();
                if (budgetSem > 0) {  // Студент был бюджетником хотя бы в одном семестре
                    if (getYearForSemester(currentSem) == year) {
                        // Проверяем, что текущий семестр >= budgetSemester
                        if (currentSem >= budgetSem && student->getMissedHours() < 12) {
                            winterScholarship = student->getScholarship();
                        }
                    } else {
                        for (int sem : yearSemesters) {
                            // Проверяем, что семестр >= budgetSemester (когда студент стал бюджетником)
                            if (sem >= budgetSem) {
                                auto it = history.find(sem);
                                if (it != history.end()) {
                                    // Используем сохраненную стипендию из истории, если есть
                                    const auto& scholarshipHistory = student->getPreviousSemesterScholarships();
                                    auto scholarshipIt = scholarshipHistory.find(sem);
                                    if (scholarshipIt != scholarshipHistory.end()) {
                                        winterScholarship = scholarshipIt->second;
                                    } else {
                                        winterScholarship = ScholarshipCalculator::calculateScholarship(it->second);
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            stats.winterTotal += winterScholarship;

            bool wasOnSummerSemester = false;
            for (int sem : yearSemesters) {
                if (sem % 2 == 0 && allSemesters.find(sem) != allSemesters.end()) {
                    wasOnSummerSemester = true;
                    break;
                }
            }

            if (wasOnSummerSemester) {
                stats.summerCount += 1;

                double summerScholarship = 0.0;
                if (isBudget) {
                    int budgetSem = student->getBudgetSemester();
                    if (budgetSem > 0) {  // Студент был бюджетником хотя бы в одном семестре
                        for (int sem : yearSemesters) {
                            if (sem % 2 == 0 && sem >= budgetSem) {  // Проверяем, что семестр >= budgetSemester
                                if (sem == currentSem && getYearForSemester(currentSem) == year) {
                                    if (student->getMissedHours() < 12) {
                                        summerScholarship = student->getScholarship();
                                    }
                                    break;
                                } else {
                                    auto it = history.find(sem);
                                    if (it != history.end()) {
                                        // Используем сохраненную стипендию из истории, если есть
                                        const auto& scholarshipHistory = student->getPreviousSemesterScholarships();
                                        auto scholarshipIt = scholarshipHistory.find(sem);
                                        if (scholarshipIt != scholarshipHistory.end()) {
                                            summerScholarship = scholarshipIt->second;
                                        } else {
                                            summerScholarship =
                                                ScholarshipCalculator::calculateScholarship(it->second);
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                stats.summerTotal += summerScholarship;
            }
        }
    }

    QColor defaultTextColor(234, 234, 234);
    QFont itemFont;
    itemFont.setPointSize(11);

    for (const auto& entry : yearStats) {
        int year = entry.first;
        const YearStats& stats = entry.second;

        int winterRow = table->rowCount();
        table->insertRow(winterRow);

        QString winterLabel = QString("%1 (Winter)").arg(year);
        QTableWidgetItem* winterItem = new QTableWidgetItem(winterLabel);
        winterItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        winterItem->setForeground(QBrush(defaultTextColor));
        winterItem->setFont(itemFont);
        winterItem->setFlags(winterItem->flags() & ~Qt::ItemIsSelectable);
        table->setItem(winterRow, 0, winterItem);

        QTableWidgetItem* winterCountItem = new QTableWidgetItem(QString::number(stats.winterCount));
        winterCountItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        winterCountItem->setForeground(QBrush(defaultTextColor));
        winterCountItem->setFont(itemFont);
        winterCountItem->setFlags(winterCountItem->flags() & ~Qt::ItemIsSelectable);
        table->setItem(winterRow, 1, winterCountItem);

        QTableWidgetItem* winterTotalItem =
            new QTableWidgetItem(QString::number(stats.winterTotal, 'f', 2));
        winterTotalItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        if (stats.winterTotal > 0) {
            winterTotalItem->setForeground(QBrush(QColor(76, 175, 80)));
            QFont boldFont = itemFont;
            boldFont.setBold(true);
            winterTotalItem->setFont(boldFont);
        } else {
            winterTotalItem->setForeground(QBrush(QColor(158, 158, 158)));
            winterTotalItem->setFont(itemFont);
        }
        winterTotalItem->setFlags(winterTotalItem->flags() & ~Qt::ItemIsSelectable);
        table->setItem(winterRow, 2, winterTotalItem);

        int summerRow = table->rowCount();
        table->insertRow(summerRow);

        QString summerLabel = QString("%1 (Summer)").arg(year);
        QTableWidgetItem* summerItem = new QTableWidgetItem(summerLabel);
        summerItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        summerItem->setForeground(QBrush(defaultTextColor));
        summerItem->setFont(itemFont);
        summerItem->setFlags(summerItem->flags() & ~Qt::ItemIsSelectable);
        table->setItem(summerRow, 0, summerItem);

        QTableWidgetItem* summerCountItem =
            new QTableWidgetItem(QString::number(stats.summerCount));
        summerCountItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        summerCountItem->setForeground(QBrush(defaultTextColor));
        summerCountItem->setFont(itemFont);
        summerCountItem->setFlags(summerCountItem->flags() & ~Qt::ItemIsSelectable);
        table->setItem(summerRow, 1, summerCountItem);

        QTableWidgetItem* summerTotalItem =
            new QTableWidgetItem(QString::number(stats.summerTotal, 'f', 2));
        summerTotalItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        if (stats.summerTotal > 0) {
            summerTotalItem->setForeground(QBrush(QColor(76, 175, 80)));
            QFont boldFont = itemFont;
            boldFont.setBold(true);
            summerTotalItem->setFont(boldFont);
        } else {
            summerTotalItem->setForeground(QBrush(QColor(158, 158, 158)));
            summerTotalItem->setFont(itemFont);
        }
        summerTotalItem->setFlags(summerTotalItem->flags() & ~Qt::ItemIsSelectable);
        table->setItem(summerRow, 2, summerTotalItem);
    }

    table->resizeColumnsToContents();
}

int StudentStatisticsUpdater::getYearForSemester(int semester) const {
    return 2026 - ((semester + 1) / 2);
}

int StudentStatisticsUpdater::getYearForSemester(int semester, int admissionYear) const {
    if (semester % 2 == 1) {
        return admissionYear + (semester - 1) / 2;
    }
    return admissionYear + semester / 2;
}
