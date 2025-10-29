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

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addStudent();
    void searchByName();
    void searchByAverageGradeEqual();
    void onCourseChanged(int course);
    void showAllStudents();
    void clearDatabase();
    void updateStudentTable(const std::vector<std::shared_ptr<Student>>& studentList);
    void editSelectedStudent();
    void deleteSelectedStudent();

private:
    void setupUI();
    void createTabs();
    void createAddStudentTab(QWidget* tabWidget);
    void createSearchTab(QWidget* tabWidget);
    void createStudentTable();
    void createActionButtons();
    void updateStatistics();
    void updateSemesterRange(int course);

    StudentDatabase database;

    QTabWidget* tabWidget;

    QLineEdit* nameEdit;
    QLineEdit* surnameEdit;
    QSpinBox* courseSpinBox;
    QSpinBox* semesterSpinBox;
    QDoubleSpinBox* averageGradeSpinBox;
    QComboBox* fundingCombo;
    QPushButton* addButton;

    QLineEdit* searchNameEdit;
    QPushButton* searchByNameButton;
    QDoubleSpinBox* exactGradeSpinBox;
    QPushButton* searchByExactGradeButton;

    QTableWidget* studentTable;

    QPushButton* editSelectedButton;
    QPushButton* deleteSelectedButton;
    QPushButton* clearButton;

    QLabel* statisticsLabel;
    std::vector<std::shared_ptr<Student>> currentView;
};

#endif 

