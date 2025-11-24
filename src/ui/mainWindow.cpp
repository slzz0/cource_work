#include "ui/mainWindow.h"

#include <QApplication>
#include <QBrush>
#include <QDate>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMouseEvent>
#include <QStringList>
#include <QVBoxLayout>
#include <algorithm>
#include <iomanip>
#include <map>
#include <numeric>
#include <random>
#include <set>
#include <sstream>
#include <vector>

#include "exceptions/exceptions.h"
#include "ui/studentDialogBuilder.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUI();
    historyDialog = std::make_unique<StudentHistoryDialog>(this);

    database.setFilename("students.txt");

    try {
        database.loadFromFile();
        // Fill missing previous semester grades for all students
        fillMissingHistoryGrades();
    } catch (const FileIOException& e) {
        QMessageBox::warning(
            this, "File Error",
            QString("Could not load file: %1\nStarting with empty database.").arg(e.what()));
    } catch (const ParseException& e) {
        QMessageBox::warning(this, "Validation Error",
                             QString("Some students were skipped due to invalid "
                                     "data:\n\n%1\n\nPlease check the file and fix the errors.")
                                 .arg(e.what()));
    }

    showAllStudents();
    updateStatistics();
}

MainWindow::~MainWindow() = default;

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    if (obj == studentTable->horizontalHeader() && event->type() == QEvent::MouseButtonPress) {
        return true;
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::setupUI() {
    setWindowTitle("Scholarship Management System");
    setMinimumSize(1400, 800);
    resize(1400, 800);

    setStyleSheet(
        "QMainWindow { background-color: #1e1e1e; }"
        "QWidget { background-color: #1e1e1e; color: #ffffff; }"
        "QTabWidget::pane { border: 1px solid #3d3d3d; background-color: #2d2d2d; }"
        "QTabBar::tab { background-color: #2d2d2d; color: #ffffff; padding: 10px 20px; "
        "margin-right: 2px; border-top-left-radius: 5px; border-top-right-radius: 5px; }"
        "QTabBar::tab:selected { background-color: #0d7377; color: #ffffff; }"
        "QTabBar::tab:hover { background-color: #14a085; }"
        "QLabel { color: #ffffff; }");

    auto centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    createTabs();
    mainLayout->addWidget(tabWidget, 1);

    updateStatistics();
}

void MainWindow::createTabs() {
    tabWidget = new QTabWidget(this);
    auto studentsTab = new QWidget(this);
    auto statisticsTab = new QWidget(this);

    createStudentsTab(studentsTab);
    createStatisticsTab(statisticsTab);

    tabWidget->addTab(studentsTab, "Students");
    tabWidget->addTab(statisticsTab, "Statistics");
}

void MainWindow::createStudentsTab(QWidget* tab) {
    auto mainLayout = new QVBoxLayout(tab);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // Search panel
    auto searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(10);

    auto searchLabel = new QLabel("Search:", this);
    searchLabel->setStyleSheet("font-weight: bold; font-size: 12px; color: #ffffff;");
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Enter student name or surname...");
    searchEdit->setMinimumWidth(300);
    searchEdit->setStyleSheet(
        "QLineEdit { "
        "padding: 8px; "
        "border: 2px solid #3d3d3d; "
        "border-radius: 5px; "
        "font-size: 12px; "
        "background-color: #2d2d2d; "
        "color: #EAEAEA; "
        "}"
        "QLineEdit:focus { "
        "border-color: #00ADB5; "
        "background-color: #252525; "
        "}"
        "QLineEdit::placeholder { "
        "color: #888888; "
        "}");

    searchButton = new QPushButton("Search", this);
    searchButton->setStyleSheet(
        "QPushButton {"
        "background-color: #0d7377;"
        "color: white;"
        "padding: 8px 20px;"
        "border-radius: 5px;"
        "font-weight: bold;"
        "font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #14a085; }"
        "QPushButton:pressed { background-color: #0a5d61; }");
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::searchStudent);
    connect(searchEdit, &QLineEdit::returnPressed, this, &MainWindow::searchStudent);

    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchButton);
    searchLayout->addStretch();

    mainLayout->addLayout(searchLayout);

    // Table and buttons panel - side by side
    auto tableButtonsLayout = new QHBoxLayout();
    tableButtonsLayout->setSpacing(15);

    // Create table first
    createStudentTable();
    tableButtonsLayout->addWidget(studentTable, 51);  // Table takes 51 parts of space (2.55:1 ratio)

    // Right side - Buttons column in a styled container
    auto buttonsGroup = new QGroupBox("Actions", this);
    buttonsGroup->setStyleSheet(
        "QGroupBox {"
        "font-weight: bold;"
        "font-size: 13px;"
        "border: 2px solid #0d7377;"
        "border-radius: 8px;"
        "margin-top: 10px;"
        "padding-top: 15px;"
        "background-color: #2d2d2d;"
        "color: #ffffff;"
        "}"
        "QGroupBox::title {"
        "subcontrol-origin: margin;"
        "left: 10px;"
        "padding: 0 5px;"
        "color: #14a085;"
        "}");

    auto buttonsLayout = new QVBoxLayout(buttonsGroup);
    buttonsLayout->setSpacing(10);
    buttonsLayout->setContentsMargins(12, 18, 12, 12);

    // Common button style
    QString buttonStyle =
        "QPushButton {"
        "background-color: #0d7377;"
        "color: white;"
        "padding: 9px 20px;"
        "border-radius: 6px;"
        "font-weight: bold;"
        "font-size: 12px;"
        "min-width: 140px;"
        "}"
        "QPushButton:hover { "
        "background-color: #14a085; "
        "}"
        "QPushButton:pressed { "
        "background-color: #0a5d61; "
        "}"
        "QPushButton:disabled { "
        "background-color: #3d3d3d; "
        "color: #666; "
        "}";

    // Add student button
    addStudentButton = new QPushButton("Add Student", this);
    addStudentButton->setStyleSheet(buttonStyle);
    connect(addStudentButton, &QPushButton::clicked, this, &MainWindow::addStudent);
    buttonsLayout->addWidget(addStudentButton);

    // Calculate scholarships button
    calculateButton = new QPushButton("Calculate Scholarships", this);
    calculateButton->setStyleSheet(buttonStyle);
    connect(calculateButton, &QPushButton::clicked, this, &MainWindow::calculateAllScholarships);
    buttonsLayout->addWidget(calculateButton);

    // Apply the same font as buttons to the table
    QFont tableFont = addStudentButton->font();
    tableFont.setBold(false);
    studentTable->setFont(tableFont);
    if (auto header = studentTable->horizontalHeader()) {
        QFont headerFont = tableFont;
        headerFont.setBold(true);
        header->setFont(headerFont);
    }

    buttonsLayout->addStretch();
    tableButtonsLayout->addWidget(buttonsGroup, 20);  // Buttons take 20 parts of space (51:20 = 2.55:1 ratio with table)

    mainLayout->addLayout(tableButtonsLayout, 1);

    recalculationWarning = new QLabel(
        "WARNING: A new student has been added, or a student has been transferred to a "
        "budget/paid. Please recalculate scholarships.",
        this);
    recalculationWarning->setStyleSheet(
        "QLabel {"
        "background-color: #DC143C;"
        "color: white;"
        "padding: 10px;"
        "border-radius: 5px;"
        "font-weight: bold;"
        "font-size: 13px;"
        "}");
    recalculationWarning->setAlignment(Qt::AlignCenter);
    recalculationWarning->setVisible(false);
    mainLayout->addWidget(recalculationWarning);
}

void MainWindow::createStatisticsTab(QWidget* tab) {
    auto mainLayout = new QVBoxLayout(tab);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // General Statistics
    auto statsGroup = new QGroupBox("General Statistics", this);
    statsGroup->setStyleSheet(
        "QGroupBox {"
        "font-weight: bold;"
        "font-size: 14px;"
        "border: 2px solid #0d7377;"
        "border-radius: 8px;"
        "margin-top: 10px;"
        "padding-top: 15px;"
        "color: #ffffff;"
        "}"
        "QGroupBox::title {"
        "subcontrol-origin: margin;"
        "left: 10px;"
        "padding: 0 5px;"
        "color: #14a085;"
        "}");
    auto statsLayout = new QGridLayout(statsGroup);

    totalStudentsLabel = new QLabel("Total Students: 0", this);
    budgetStudentsLabel = new QLabel("Budget Students: 0", this);
    paidStudentsLabel = new QLabel("Paid Students: 0", this);
    totalScholarshipLabel = new QLabel("Total Scholarships: 0.00 BYN", this);

    QString labelStyle = "font-size: 13px; padding: 8px; color: #ffffff;";
    totalStudentsLabel->setStyleSheet(labelStyle);
    budgetStudentsLabel->setStyleSheet(labelStyle);
    paidStudentsLabel->setStyleSheet(labelStyle);
    totalScholarshipLabel->setStyleSheet(labelStyle + "font-weight: bold; color: #4CAF50;");

    statsLayout->addWidget(totalStudentsLabel, 0, 0);
    statsLayout->addWidget(budgetStudentsLabel, 0, 1);
    statsLayout->addWidget(paidStudentsLabel, 1, 0);
    statsLayout->addWidget(totalScholarshipLabel, 1, 1);

    mainLayout->addWidget(statsGroup);

    // Year/Session Statistics Table
    auto semesterStatsLabel = new QLabel("Statistics by Year/Session:", this);
    semesterStatsLabel->setStyleSheet(
        "font-weight: bold; font-size: 14px; color: #14a085; margin-top: 10px;");
    mainLayout->addWidget(semesterStatsLabel);

    semesterStatsTable = new QTableWidget(this);
    semesterStatsTable->setColumnCount(3);
    semesterStatsTable->setHorizontalHeaderLabels(QStringList()
                                                  << "Year / Session" << "Students Count"
                                                  << "Total Scholarship (BYN)");
    semesterStatsTable->verticalHeader()->setVisible(false);
    semesterStatsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    semesterStatsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    semesterStatsTable->setAlternatingRowColors(true);
    semesterStatsTable->horizontalHeader()->setStretchLastSection(true);

    semesterStatsTable->setStyleSheet(
        "QTableWidget {"
        "gridline-color: #2d2d2d;"
        "background-color: #1B1B1B;"
        "alternate-background-color: #1E1E1E;"
        "border: 2px solid #0d7377;"
        "border-radius: 8px;"
        "}"
        "QTableWidget::item {"
        "padding: 14px 12px;"
        "border: none;"
        "color: #EAEAEA;"
        "font-size: 11px;"
        "}"
        "QTableWidget::item:selected {"
        "background-color: #0d7377 !important;"
        "}"
        "QTableWidget::item:selected:alternate {"
        "background-color: #0d7377 !important;"
        "}"
        "QTableWidget::item:alternate {"
        "background-color: #1E1E1E;"
        "}"
        "QHeaderView::section {"
        "background-color: #2A2A2A;"
        "color: #EAEAEA;"
        "padding: 16px 14px;"
        "font-weight: 700;"
        "border: none;"
        "border-bottom: 2px solid #0d7377;"
        "font-size: 12px;"
        "min-height: 40px;"
        "}");

    mainLayout->addWidget(semesterStatsTable, 1);
}

void MainWindow::createStudentTable() {
    studentTable = new QTableWidget(this);
    tableManager = std::make_unique<StudentTableManager>(studentTable, this);
    tableManager->configure(this);
    // Connect action button signals from table manager to slots
    connect(tableManager.get(), &StudentTableManager::editStudentRequested, this, [this](int row) {
        if (row >= 0 && row < static_cast<int>(currentView.size())) {
            studentTable->setCurrentCell(row, 0);
            editSelectedStudent();
        }
    });

    connect(tableManager.get(), &StudentTableManager::deleteStudentRequested, this,
            [this](int row) {
                if (row >= 0 && row < static_cast<int>(currentView.size())) {
                    studentTable->setCurrentCell(row, 0);
                    deleteSelectedStudent();
                }
            });

    connect(tableManager.get(), &StudentTableManager::viewHistoryRequested, this, [this](int row) {
        if (row >= 0 && row < static_cast<int>(currentView.size())) {
            studentTable->setCurrentCell(row, 0);
            showStudentHistory();
        }
    });
}

void MainWindow::addStudent() {
    StudentDialogBuilder builder(this);
    StudentDialogResult result = builder.showAddDialog();

    if (!result.isAccepted()) {
        return;
    }

    try {
        int semester = result.getSemester();
        int course = (semester - 1) / 2 + 1;

        auto student = std::make_shared<Student>(
            result.getName().toStdString(), result.getSurname().toStdString(), course, semester,
            result.getAverageGrade(), result.isBudget());

        student->setMissedHours(result.getMissedHours());
        student->setHasSocialScholarship(result.hasSocialScholarship());

        historyGradeGenerator.ensureHistoryForNewStudent(*student, semester);

        database.addStudent(student);

        if (scholarshipsCalculated) {
            scholarshipsNeedRecalculation = true;
            recalculationWarning->setVisible(true);
        }

        showAllStudents();
        updateStatistics();

        saveDatabaseToFile();

        QMessageBox::information(this, "Success", "Student added successfully!");
    } catch (const ValidationException& e) {
        QMessageBox::critical(this, "Validation Error", e.what());
    } catch (const ScholarshipException& e) {
        QMessageBox::critical(this, "Error", e.what());
    } catch (const std::runtime_error& e) {
        QMessageBox::critical(this, "Unexpected Error", e.what());
    }
}

void MainWindow::searchStudent() {
    QString searchText = searchEdit->text().trimmed();

    if (searchText.isEmpty()) {
        showAllStudents();
        return;
    }

    try {
        std::vector<std::shared_ptr<Student>> results =
            database.searchByName(searchText.toStdString());

        if (results.empty()) {
            throw StudentNotFoundException(searchText.toStdString());
        }

        updateStudentTable(results);
    } catch (const StudentNotFoundException& e) {
        QMessageBox::information(this, "Not Found", e.what());
        showAllStudents();
    } catch (const std::runtime_error& e) {
        QMessageBox::critical(this, "Search Error", e.what());
    }
}

void MainWindow::calculateAllScholarships() {
    auto allStudents = database.getAllStudents();
    int count = 0;

    for (const auto& student : allStudents) {
        student->recalculateScholarship();
        count++;
    }

    scholarshipsCalculated = true;
    scholarshipsNeedRecalculation = false;
    recalculationWarning->setVisible(false);

    database.saveToFile();
    showAllStudents();
    updateStatistics();

    QMessageBox::information(
        this, "Success",
        QString("Successfully calculated scholarships for %1 students.").arg(count));
}

void MainWindow::showAllStudents() {
    std::vector<std::shared_ptr<Student>> allStudents = database.getAllStudents();
    updateStudentTable(allStudents);
}

void MainWindow::updateStudentTable(const std::vector<std::shared_ptr<Student>>& studentList) {
    currentView = studentList;
    if (tableManager) {
        tableManager->populate(studentList, scholarshipsCalculated);
    }
}

void MainWindow::updateStatistics() {
    auto allStudents = database.getAllStudents();
    statisticsUpdater.updateGeneralStatistics(allStudents, totalStudentsLabel, budgetStudentsLabel,
                                              paidStudentsLabel, totalScholarshipLabel);
    if (semesterStatsTable) {
        statisticsUpdater.updateSemesterTable(allStudents, semesterStatsTable);
    }
}

void MainWindow::editSelectedStudent() {
    int row = studentTable->currentRow();
    if (row < 0 || row >= static_cast<int>(currentView.size())) {
        QMessageBox::warning(this, "Error", "Please select a student to edit.");
        return;
    }

    auto student = currentView[row];
    if (!student) return;

    StudentDialogBuilder builder(this);
    StudentDialogResult result = builder.showEditDialog(student);

    if (!result.isAccepted()) {
        return;
    }

    try {
        int oldSemester = student->getSemester();
        double oldGrade = student->getAverageGrade();
        int newSemester = result.getSemester();

        student->setName(result.getName().toStdString());
        student->setSurname(result.getSurname().toStdString());
        student->setSemester(newSemester);
        student->setAverageGrade(result.getAverageGrade());
        student->setIsBudget(result.isBudget());
        student->setMissedHours(result.getMissedHours());
        student->setHasSocialScholarship(result.hasSocialScholarship());

        historyGradeGenerator.handleSemesterChange(*student, oldSemester, oldGrade, newSemester);

        if (scholarshipsCalculated) {
            scholarshipsNeedRecalculation = true;
            recalculationWarning->setVisible(true);
        }

        showAllStudents();
        updateStatistics();

        try {
            database.saveToFile();
        } catch (const FileWriteException& e) {
            QMessageBox::warning(this, "File Error", e.what());
        }

        QMessageBox::information(this, "Success", "Student data updated.");
    } catch (const ValidationException& e) {
        QMessageBox::critical(this, "Validation Error", e.what());
    } catch (const ScholarshipException& e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

void MainWindow::deleteSelectedStudent() {
    int row = studentTable->currentRow();
    if (row < 0 || row >= static_cast<int>(currentView.size())) {
        QMessageBox::warning(this, "Error", "Please select a student to delete.");
        return;
    }
    auto student = currentView[row];
    if (!student) return;
    int ret = QMessageBox::question(this, "Confirm",
                                    QString("Are you sure you want to delete student %1 %2?")
                                        .arg(QString::fromStdString(student->getName()))
                                        .arg(QString::fromStdString(student->getSurname())),
                                    QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        try {
            database.removeStudentPtr(student);

            if (scholarshipsCalculated) {
                scholarshipsNeedRecalculation = true;
                recalculationWarning->setVisible(true);
            }

            showAllStudents();
            updateStatistics();

            try {
                database.saveToFile();
            } catch (const FileWriteException& e) {
                QMessageBox::warning(this, "File Error", e.what());
            }

            QMessageBox::information(this, "Success", "Student deleted.");
        } catch (const DatabaseException& e) {
            QMessageBox::critical(this, "Database Error", e.what());
        }
    }
}

void MainWindow::showStudentHistory() {
    int row = studentTable->currentRow();
    if (row < 0 || row >= static_cast<int>(currentView.size())) {
        QMessageBox::warning(this, "Error", "Please select a student to view history.");
        return;
    }

    auto student = currentView[row];
    if (!student) return;

    if (!historyDialog) {
        historyDialog = std::make_unique<StudentHistoryDialog>(this);
    }
    historyDialog->showHistory(student);
}

void MainWindow::fillMissingHistoryGrades() {
    historyGradeGenerator.fillMissingHistoryForAll(database.getAllStudents());
}

void MainWindow::saveDatabaseToFile() {
    try {
        database.saveToFile();
    } catch (const FileWriteException& e) {
        QMessageBox::warning(this, "File Error", e.what());
    }
}