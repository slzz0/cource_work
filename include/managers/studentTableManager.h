#ifndef STUDENTTABLEMANAGER_H
#define STUDENTTABLEMANAGER_H

#include <memory>
#include <vector>

#include <QObject>

class QTableWidget;
class QTableWidgetItem;
class QObject;
class Student;

class StudentTableManager : public QObject {
    Q_OBJECT

public:
    explicit StudentTableManager(QTableWidget* table, QObject* parent = nullptr);

    void configure(QObject* eventFilterOwner);
    void populate(const std::vector<std::shared_ptr<Student>>& students, bool scholarshipsCalculated);
    void updateRowNumbers();

signals:
    void editStudentRequested(int row);
    void deleteStudentRequested(int row);
    void viewHistoryRequested(int row);

private slots:
    void updateSelectionVisual() const;
    void onEditClicked();
    void onDeleteClicked();
    void onViewClicked();

private:
    QTableWidget* table = nullptr;

    void ensureScholarshipColumn(bool scholarshipsCalculated);
    void applyMissedHoursStyling() const;
    QWidget* createActionButtons(int row);
    void setupColumnWidths(bool scholarshipsCalculated);
    void createRowItems(int row, int& rowNum, const std::shared_ptr<Student>& student,
                       const QFont& itemFont, const QColor& defaultTextColor,
                       bool scholarshipsCalculated);
    QTableWidgetItem* createNumberItem(int rowNum, const QFont& itemFont);
    QTableWidgetItem* createMissedHoursItem(const std::shared_ptr<Student>& student,
                                            const QFont& itemFont, const QColor& defaultTextColor);
    QTableWidgetItem* createScholarshipItem(double scholarship, const QFont& itemFont);
    void applyItemForegroundColor(QTableWidgetItem* item, int col, const QColor& defaultColor) const;
    bool shouldSkipColumn(int col) const;
};

#endif