#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QVBoxLayout>

#include "scholarshipCalculator.h"
#include "studentDatabase.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
    // Override eventFilter to block sorting clicks on specific columns
    bool eventFilter(QObject* obj, QEvent* event) override;

   private slots:
    void addStudent();
    void searchStudent();
    void calculateAllScholarships();
    void onCourseChanged(int course);
    void showAllStudents();
    void updateStudentTable(const std::vector<std::shared_ptr<Student>>& studentList);
    void editSelectedStudent();
    void deleteSelectedStudent();
    void changeFundingType();
    void showStudentHistory();

   private:
    void setupUI();
    void createTabs();
    void createStudentsTab(QWidget* tabWidget);
    void createStatisticsTab(QWidget* tabWidget);
    void createStudentTable();
    void updateStatistics();
    void updateSemesterRange(int course);
    void showStudentDetailsDialog(std::shared_ptr<Student> student);
    void updateRowNumbers();

    StudentDatabase database;

    QTabWidget* tabWidget;

    // Students Tab
    QTableWidget* studentTable;
    QLineEdit* searchEdit;
    QPushButton* searchButton;
    QPushButton* addStudentButton;
    QPushButton* editButton;
    QPushButton* deleteButton;
    QPushButton* changeFundingButton;
    QPushButton* viewHistoryButton;

    // Add Student Dialog fields
    QLineEdit* nameEdit;
    QLineEdit* surnameEdit;
    QSpinBox* courseSpinBox;
    QSpinBox* semesterSpinBox;
    QDoubleSpinBox* averageGradeSpinBox;
    QComboBox* fundingCombo;
    QSpinBox* missedHoursSpinBox;
    QCheckBox* socialScholarshipCheckBox;

    // Statistics Tab
    QLabel* totalStudentsLabel;
    QLabel* budgetStudentsLabel;
    QLabel* paidStudentsLabel;
    QLabel* totalScholarshipLabel;
    QPushButton* calculateButton;
    QTextEdit* statisticsText;

    std::vector<std::shared_ptr<Student>> currentView;
    bool scholarshipsCalculated = false;
    bool scholarshipsNeedRecalculation = false;
    QLabel* recalculationWarning;
};

#endif