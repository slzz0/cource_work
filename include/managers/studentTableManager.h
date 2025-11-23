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
    void updateSelectionVisual();
    void onEditClicked();
    void onDeleteClicked();
    void onViewClicked();

private:
    QTableWidget* table = nullptr;

    void ensureScholarshipColumn(bool scholarshipsCalculated);
    void applyMissedHoursStyling();
    QWidget* createActionButtons(int row);
};

#endif