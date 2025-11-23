#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <memory>

#include "historyGradeGenerator.h"
#include "scholarshipCalculator.h"
#include "studentDatabase.h"
#include "studentHistoryDialog.h"
#include "studentStatisticsUpdater.h"
#include "studentTableManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    bool eventFilter(QObject* obj, QEvent* event) override;

   private slots:
    void addStudent();
    void searchStudent();
    void calculateAllScholarships();
    void showAllStudents();
    void updateStudentTable(const std::vector<std::shared_ptr<Student>>& studentList);
    void editSelectedStudent();
    void deleteSelectedStudent();
    void showStudentHistory();

   private:
    void setupUI();
    void createTabs();
    void createStudentsTab(QWidget* tabWidget);
    void createStatisticsTab(QWidget* tabWidget);
    void createStudentTable();
    void updateStatistics();
    void fillMissingHistoryGrades();

    StudentDatabase database;

    QTabWidget* tabWidget = nullptr;

    QTableWidget* studentTable = nullptr;
    QLineEdit* searchEdit = nullptr;
    QPushButton* searchButton = nullptr;
    QPushButton* addStudentButton = nullptr;

    QLabel* totalStudentsLabel = nullptr;
    QLabel* budgetStudentsLabel = nullptr;
    QLabel* paidStudentsLabel = nullptr;
    QLabel* totalScholarshipLabel = nullptr;
    QPushButton* calculateButton = nullptr;
    QTableWidget* semesterStatsTable = nullptr;

    std::vector<std::shared_ptr<Student>> currentView;
    bool scholarshipsCalculated = false;
    bool scholarshipsNeedRecalculation = false;
    QLabel* recalculationWarning = nullptr;

    std::unique_ptr<StudentTableManager> tableManager;
    StudentStatisticsUpdater statisticsUpdater;
    HistoryGradeGenerator historyGradeGenerator;
    std::unique_ptr<StudentHistoryDialog> historyDialog;
};

#endif