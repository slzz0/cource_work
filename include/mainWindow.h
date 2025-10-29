#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QLabel>
#include <QGroupBox>
#include <QTabWidget>
#include <QMessageBox>
#include <QHeaderView>
#include <QComboBox>
#include "studentDatabase.h"
#include "scholarshipCalculator.h"

/**
 * @brief Main window class for Scholarship Management System
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addStudent();
    void searchByName();
    void searchByAverageGradeEqual();
    void showAllStudents();
    void calculateAllScholarships();
    void clearDatabase();
    void updateStudentTable(const std::vector<std::shared_ptr<Student>>& studentList);

private:
    void setupUI();
    void createTabs();
    void createAddStudentTab(QWidget* tabWidget);
    void createSearchTab(QWidget* tabWidget);
    void createStudentTable();
    void createActionButtons();
    void updateStatistics();

    // Database
    StudentDatabase database;

    // UI Components - Tabs
    QTabWidget* tabWidget;

    // UI Components - Add Student Section
    QLineEdit* nameEdit;
    QLineEdit* surnameEdit;
    QSpinBox* courseSpinBox;
    QSpinBox* semesterSpinBox;
    QDoubleSpinBox* averageGradeSpinBox;
    QComboBox* fundingCombo;
    QPushButton* addButton;

    // UI Components - Search Section
    QLineEdit* searchNameEdit;
    QPushButton* searchByNameButton;
    QDoubleSpinBox* exactGradeSpinBox;
    QPushButton* searchByExactGradeButton;

    // UI Components - Table
    QTableWidget* studentTable;

    // UI Components - Action Buttons
    QPushButton* showAllButton;
    QPushButton* calculateScholarshipsButton;
    QPushButton* clearButton;

    // Statistics
    QLabel* statisticsLabel;
};

#endif // MAINWINDOW_H

