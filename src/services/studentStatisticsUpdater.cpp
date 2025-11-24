#include "services/studentStatisticsUpdater.h"

#include <QBrush>
#include <QFont>
#include <QLabel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <ranges>
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

std::set<int> StudentStatisticsUpdater::collectAllSemesters(const std::shared_ptr<Student>& student) const {
    std::set<int> allSemesters;
    allSemesters.insert(student->getSemester());
    const auto& history = student->getPreviousSemesterGrades();
    for (const auto& [sem, grade] : history) {
        allSemesters.insert(sem);
    }
    return allSemesters;
}

std::vector<int> StudentStatisticsUpdater::getYearSemesters(int year) const {
    std::vector<int> yearSemesters;
    for (int sem = 1; sem <= 8; ++sem) {
        if (getYearForSemester(sem) == year) {
            yearSemesters.push_back(sem);
        }
    }
    return yearSemesters;
}

bool StudentStatisticsUpdater::wasStudentInYear(const std::set<int>& allSemesters,
                                                const std::vector<int>& yearSemesters) const {
    return std::ranges::any_of(yearSemesters, [&allSemesters](int sem) {
        return allSemesters.contains(sem);
    });
}

bool StudentStatisticsUpdater::wasOnSummerSemester(const std::set<int>& allSemesters,
                                                   const std::vector<int>& yearSemesters) const {
    return std::ranges::any_of(yearSemesters, [&allSemesters](int sem) {
        return sem % 2 == 0 && allSemesters.contains(sem);
    });
}

double StudentStatisticsUpdater::getScholarshipFromHistory(const std::shared_ptr<Student>& student,
                                                           int sem) const {
    // Сначала проверяем сохраненную историю стипендий
    const auto& scholarshipHistory = student->getPreviousSemesterScholarships();
    if (auto scholarshipIt = scholarshipHistory.find(sem); scholarshipIt != scholarshipHistory.end()) {
        return scholarshipIt->second;
    }
    // Если стипендия не найдена в истории, проверяем, был ли студент бюджетником в этом семестре
    if (int budgetSem = student->getBudgetSemester(); budgetSem > 0 && sem >= budgetSem) {
        // Студент был бюджетником в этом семестре, вычисляем стипендию из оценки
        const auto& history = student->getPreviousSemesterGrades();
        if (auto it = history.find(sem); it != history.end()) {
            return ScholarshipCalculator::calculateScholarship(it->second);
        }
    }
    return 0.0;
}

double StudentStatisticsUpdater::calculateWinterScholarship(const std::shared_ptr<Student>& student,
                                                           int year, const std::vector<int>& yearSemesters,
                                                           int currentSem) const {
    int budgetSem = student->getBudgetSemester();
    if (budgetSem <= 0) {
        return 0.0;
    }
    
    // Для текущего семестра проверяем, является ли студент сейчас бюджетником
    if (getYearForSemester(currentSem) == year) {
        if (student->getIsBudget() && currentSem >= budgetSem && student->getMissedHours() < 12) {
            return student->getScholarship();
        }
        return 0.0;
    }
    
    // Для прошлых семестров учитываем стипендии, даже если студент сейчас платный
    // но был бюджетником в прошлых семестрах
    for (int sem : yearSemesters) {
        if (sem >= budgetSem) {
            return getScholarshipFromHistory(student, sem);
        }
    }
    return 0.0;
}

double StudentStatisticsUpdater::calculateSummerScholarship(const std::shared_ptr<Student>& student,
                                                           int year, const std::vector<int>& yearSemesters,
                                                           int currentSem) const {
    int budgetSem = student->getBudgetSemester();
    if (budgetSem <= 0) {
        return 0.0;
    }
    
    for (int sem : yearSemesters) {
        if (sem % 2 != 0 || sem < budgetSem) {
            continue;
        }
        
        // Для текущего семестра проверяем, является ли студент сейчас бюджетником
        if (sem == currentSem && getYearForSemester(currentSem) == year) {
            if (student->getIsBudget() && student->getMissedHours() < 12) {
                return student->getScholarship();
            }
            return 0.0;
        }
        
        // Для прошлых семестров учитываем стипендии, даже если студент сейчас платный
        // но был бюджетником в прошлых семестрах
        return getScholarshipFromHistory(student, sem);
    }
    return 0.0;
}

void StudentStatisticsUpdater::populateTableRows(QTableWidget* table,
                                                 const std::map<int, YearStats>& yearStats) const {
    QColor defaultTextColor(234, 234, 234);
    QFont itemFont;
    itemFont.setPointSize(11);

    for (const auto& [year, stats] : yearStats) {
        int winterRow = table->rowCount();
        table->insertRow(winterRow);

        auto winterLabel = QString("%1 (Winter)").arg(year);
        auto winterItem = new QTableWidgetItem(winterLabel);
        winterItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        winterItem->setForeground(QBrush(defaultTextColor));
        winterItem->setFont(itemFont);
        winterItem->setFlags(winterItem->flags() & ~Qt::ItemIsSelectable);
        table->setItem(winterRow, 0, winterItem);

        auto winterCountItem = new QTableWidgetItem(QString::number(stats.winterCount));
        winterCountItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        winterCountItem->setForeground(QBrush(defaultTextColor));
        winterCountItem->setFont(itemFont);
        winterCountItem->setFlags(winterCountItem->flags() & ~Qt::ItemIsSelectable);
        table->setItem(winterRow, 1, winterCountItem);

        auto winterTotalItem =
            new QTableWidgetItem(QString::number(stats.winterTotal, 'f', 2));
        winterTotalItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        if (stats.winterTotal > 0) {
            winterTotalItem->setForeground(QBrush(QColor(76, 175, 80)));
            auto boldFont = itemFont;
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

        auto summerLabel = QString("%1 (Summer)").arg(year);
        auto summerItem = new QTableWidgetItem(summerLabel);
        summerItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        summerItem->setForeground(QBrush(defaultTextColor));
        summerItem->setFont(itemFont);
        summerItem->setFlags(summerItem->flags() & ~Qt::ItemIsSelectable);
        table->setItem(summerRow, 0, summerItem);

        auto summerCountItem =
            new QTableWidgetItem(QString::number(stats.summerCount));
        summerCountItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        summerCountItem->setForeground(QBrush(defaultTextColor));
        summerCountItem->setFont(itemFont);
        summerCountItem->setFlags(summerCountItem->flags() & ~Qt::ItemIsSelectable);
        table->setItem(summerRow, 1, summerCountItem);

        auto summerTotalItem =
            new QTableWidgetItem(QString::number(stats.summerTotal, 'f', 2));
        summerTotalItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        if (stats.summerTotal > 0) {
            summerTotalItem->setForeground(QBrush(QColor(76, 175, 80)));
            auto boldFont = itemFont;
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

void StudentStatisticsUpdater::updateSemesterTable(
    const std::vector<std::shared_ptr<Student>>& students, QTableWidget* table) const {
    if (!table) return;

    table->setRowCount(0);

    std::map<int, YearStats> yearStats;
    for (int year = 2022; year <= 2025; ++year) {
        yearStats[year] = YearStats{};
    }

    for (const auto& student : students) {
        if (!student) continue;
        
        std::set<int> allSemesters = collectAllSemesters(student);
        int currentSem = student->getSemester();

        for (int year = 2022; year <= 2025; ++year) {
            std::vector<int> yearSemesters = getYearSemesters(year);
            
            if (!wasStudentInYear(allSemesters, yearSemesters)) {
                continue;
            }

            auto& stats = yearStats[year];
            stats.winterCount += 1;
            stats.winterTotal += calculateWinterScholarship(student, year, yearSemesters, currentSem);

            if (wasOnSummerSemester(allSemesters, yearSemesters)) {
                stats.summerCount += 1;
                stats.summerTotal += calculateSummerScholarship(student, year, yearSemesters, currentSem);
            }
        }
    }

    populateTableRows(table, yearStats);
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
