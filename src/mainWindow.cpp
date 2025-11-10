#include "mainWindow.h"

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

#include "exceptions.h"


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUI();

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

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    createTabs();
    mainLayout->addWidget(tabWidget, 1);

    updateStatistics();
}

void MainWindow::createTabs() {
    tabWidget = new QTabWidget(this);
    QWidget* studentsTab = new QWidget(this);
    QWidget* statisticsTab = new QWidget(this);

    createStudentsTab(studentsTab);
    createStatisticsTab(statisticsTab);

    tabWidget->addTab(studentsTab, "Students");
    tabWidget->addTab(statisticsTab, "Statistics");
}

void MainWindow::createStudentsTab(QWidget* tab) {
    QVBoxLayout* mainLayout = new QVBoxLayout(tab);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // Search panel
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(10);

    QLabel* searchLabel = new QLabel("Search:", this);
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
    QHBoxLayout* tableButtonsLayout = new QHBoxLayout();
    tableButtonsLayout->setSpacing(15);

    // Create table first
    createStudentTable();
    tableButtonsLayout->addWidget(studentTable, 3); // Table takes 3 parts of space

    // Right side - Buttons column in a styled container
    QGroupBox* buttonsGroup = new QGroupBox("Actions", this);
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
    
    QVBoxLayout* buttonsLayout = new QVBoxLayout(buttonsGroup);
    buttonsLayout->setSpacing(10);
    buttonsLayout->setContentsMargins(15, 20, 15, 15);

    // Common button style
    QString buttonStyle = 
        "QPushButton {"
        "background-color: #0d7377;"
        "color: white;"
        "padding: 10px 25px;"
        "border-radius: 6px;"
        "font-weight: bold;"
        "font-size: 12px;"
        "min-width: 160px;"
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

    // Edit student button
    editButton = new QPushButton("Edit Student", this);
    editButton->setStyleSheet(buttonStyle);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editSelectedStudent);
    buttonsLayout->addWidget(editButton);

    // Delete student button
    deleteButton = new QPushButton("Delete Student", this);
    deleteButton->setStyleSheet(buttonStyle);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedStudent);
    buttonsLayout->addWidget(deleteButton);

    // View history button
    viewHistoryButton = new QPushButton("View History", this);
    viewHistoryButton->setStyleSheet(buttonStyle);
    connect(viewHistoryButton, &QPushButton::clicked, this, &MainWindow::showStudentHistory);
    buttonsLayout->addWidget(viewHistoryButton);

    // Calculate scholarships button
    calculateButton = new QPushButton("Calculate Scholarships", this);
    calculateButton->setStyleSheet(buttonStyle);
    connect(calculateButton, &QPushButton::clicked, this, &MainWindow::calculateAllScholarships);
    buttonsLayout->addWidget(calculateButton);

    buttonsLayout->addStretch();
    tableButtonsLayout->addWidget(buttonsGroup, 1); // Buttons take 1 part of space

    mainLayout->addLayout(tableButtonsLayout, 1);

    recalculationWarning =
        new QLabel("WARNING: A new student has been added, or a student has been transferred to a budget/paid. Please recalculate scholarships.", this);
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
    QVBoxLayout* mainLayout = new QVBoxLayout(tab);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // General Statistics
    QGroupBox* statsGroup = new QGroupBox("General Statistics", this);
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
    QGridLayout* statsLayout = new QGridLayout(statsGroup);

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
    QLabel* semesterStatsLabel = new QLabel("Statistics by Year/Session:", this);
    semesterStatsLabel->setStyleSheet(
        "font-weight: bold; font-size: 14px; color: #14a085; margin-top: 10px;");
    mainLayout->addWidget(semesterStatsLabel);

    semesterStatsTable = new QTableWidget(this);
    semesterStatsTable->setColumnCount(3);
    semesterStatsTable->setHorizontalHeaderLabels(
        QStringList() << "Year / Session" << "Students Count"
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
    studentTable->setColumnCount(9);
    QStringList headers = {"#",        "Name",         "Surname",       "Course",
                           "Semester", "Funding Type", "Average Grade", "Missed Hours",
                           "Social"};
    studentTable->setHorizontalHeaderLabels(headers);

    studentTable->verticalHeader()->setVisible(false);

    studentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    studentTable->setSelectionMode(QAbstractItemView::SingleSelection);
    studentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    studentTable->setAlternatingRowColors(true);
    studentTable->horizontalHeader()->setStretchLastSection(false);
    studentTable->setSortingEnabled(false);  

    studentTable->horizontalHeader()->installEventFilter(this);
    studentTable->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "background-color: #2A2A2A;"
        "color: #EAEAEA;"
        "padding: 16px 14px;"
        "font-weight: 700;"
        "border: none;"
        "border-bottom: 2px solid #0d7377;"
        "font-size: 12px;"
        "min-height: 40px;"
        "}"
        "QHeaderView::section:first {"
        "background-color: transparent;"
        "color: #EAEAEA;"
        "border-bottom: none;"
        "}");
    studentTable->setStyleSheet(
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
        "}");
    
    // Set minimum row height for better readability
    studentTable->verticalHeader()->setDefaultSectionSize(45);

    auto updateSelectionVisual = [this]() {
        bool hasSelection = studentTable->currentRow() >= 0;
        editButton->setEnabled(hasSelection);
        deleteButton->setEnabled(hasSelection);
        viewHistoryButton->setEnabled(hasSelection);

        int selectedRow = studentTable->currentRow();
        int colCount = studentTable->columnCount();

        for (int row = 0; row < studentTable->rowCount(); ++row) {
            bool isSelected = (row == selectedRow);
            QColor bgColor =
                isSelected ? QColor(13, 115, 119) : QColor();  
            QColor textColor = isSelected ? QColor(255, 255, 255) : QColor(234, 234, 234);

            for (int col = 1; col < colCount; ++col) {  
                QTableWidgetItem* item = studentTable->item(row, col);
                if (item && (item->flags() & Qt::ItemIsSelectable)) {
                    // Check if this is missed hours column with high missed hours (>= 12)
                    bool isHighMissedHours = (col == 7 && item->data(Qt::UserRole + 10).toBool());
                    QColor missedHoursColor;
                    if (isHighMissedHours) {
                        QVariant colorVar = item->data(Qt::UserRole + 11);
                        if (colorVar.isValid()) {
                            missedHoursColor = colorVar.value<QColor>();
                        } else {
                            missedHoursColor = QColor(255, 0, 0); // Fallback to red
                        }
                    }
                    
                    if (isSelected) {
                        item->setBackground(QBrush(bgColor));
                        if (isHighMissedHours) {
                            item->setForeground(QBrush(missedHoursColor)); // Keep red color even when selected
                        } else if (col == 9 && scholarshipsCalculated) {
                            item->setForeground(QBrush(textColor));
                        } else {
                            item->setForeground(QBrush(textColor));
                        }
                    } else {
                        item->setBackground(QBrush());
                        if (isHighMissedHours) {
                            item->setForeground(QBrush(missedHoursColor)); // Restore red color
                        } else if (col == 9 && scholarshipsCalculated) {
                            QVariant storedColor = item->data(Qt::UserRole + 1);
                            if (storedColor.isValid()) {
                                item->setForeground(QBrush(storedColor.value<QColor>()));
                            } else {
                                item->setForeground(QBrush(textColor));
                            }
                        } else {
                            item->setForeground(QBrush(textColor));
                        }
                    }
                }
            }
        }
    };

    connect(studentTable, &QTableWidget::itemSelectionChanged, this, updateSelectionVisual);
    connect(
        studentTable, &QTableWidget::currentItemChanged, this,
        [updateSelectionVisual](QTableWidgetItem*, QTableWidgetItem*) { updateSelectionVisual(); });
}

void MainWindow::addStudent() {
    QDialog dialog(this);
    dialog.setWindowTitle("Add Student");
    dialog.setMinimumWidth(400);
    dialog.setStyleSheet(
        "QDialog { background-color: #2d2d2d; color: #ffffff; }"
        "QLabel { "
        "color: #e0e0e0; "
        "font-size: 12px; "
        "background: none; "
        "background-color: rgba(0,0,0,0); "
        "border: 0px; "
        "padding: 0px; "
        "}"
        "QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox { "
        "background-color: #1e1e1e; "
        "color: #EAEAEA; "
        "border: 2px solid #0d7377; "
        "border-radius: 4px; "
        "padding: 8px; "
        "}"
        "QLineEdit::placeholder, QSpinBox::placeholder, QDoubleSpinBox::placeholder { "
        "color: #888888; "
        "}"
        "QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus { "
        "border-color: #14a085; "
        "background-color: #252525; "
        "}"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { "
        "background-color: #1e1e1e; "
        "color: #EAEAEA; "
        "selection-background-color: #0d7377; "
        "}");

    QFormLayout* form = new QFormLayout(&dialog);
    form->setSpacing(15);
    form->setContentsMargins(20, 20, 20, 20);

    nameEdit = new QLineEdit(&dialog);
    nameEdit->setPlaceholderText("Enter name");

    surnameEdit = new QLineEdit(&dialog);
    surnameEdit->setPlaceholderText("Enter surname");

    semesterSpinBox = new QSpinBox(&dialog);
    semesterSpinBox->setRange(1, 8);
    semesterSpinBox->setValue(1);
    // Prevent selecting semester 2
    connect(semesterSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        if (value == 2) {
            semesterSpinBox->setValue(3);
        }
    });

    averageGradeSpinBox = new QDoubleSpinBox(&dialog);
    averageGradeSpinBox->setRange(0.0, 10.0);
    averageGradeSpinBox->setDecimals(2);
    averageGradeSpinBox->setSingleStep(0.1);
    averageGradeSpinBox->setValue(5.0);

    fundingCombo = new QComboBox(&dialog);
    fundingCombo->addItem("Budget");
    fundingCombo->addItem("Paid");

    missedHoursSpinBox = new QSpinBox(&dialog);
    missedHoursSpinBox->setRange(0, 100);
    missedHoursSpinBox->setValue(0);
    missedHoursSpinBox->setToolTip("If >= 12 hours, student will lose scholarship");

    socialScholarshipCheckBox = new QCheckBox(&dialog);
    socialScholarshipCheckBox->setText("Social Scholarship");
    socialScholarshipCheckBox->setStyleSheet("color: #e0e0e0;");

    form->addRow("Name:", nameEdit);
    form->addRow("Surname:", surnameEdit);
    form->addRow("Semester:", semesterSpinBox);
    form->addRow("Average Grade:", averageGradeSpinBox);
    form->addRow("Funding Type:", fundingCombo);
    form->addRow("Missed Hours:", missedHoursSpinBox);
    form->addRow("", socialScholarshipCheckBox);

    QDialogButtonBox* bb =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    bb->button(QDialogButtonBox::Ok)->setText("Add");
    bb->button(QDialogButtonBox::Cancel)->setText("Cancel");
    bb->button(QDialogButtonBox::Ok)
        ->setStyleSheet(
            "QPushButton { background-color: #4CAF50; color: white; padding: 8px 20px; "
            "border-radius: 4px; font-weight: bold; }"
            "QPushButton:hover { background-color: #45a049; }");
    bb->button(QDialogButtonBox::Cancel)
        ->setStyleSheet(
            "QPushButton { background-color: #f44336; color: white; padding: 8px 20px; "
            "border-radius: 4px; font-weight: bold; }"
            "QPushButton:hover { background-color: #da190b; }");
    form->addRow(bb);

    connect(bb, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString name = nameEdit->text().trimmed();
        QString surname = surnameEdit->text().trimmed();

        if (name.isEmpty() || surname.isEmpty()) {
            QMessageBox::warning(this, "Error", "Please enter both name and surname.");
            return;
        }

        try {
            bool isBudget = (fundingCombo->currentText() == "Budget");
            int semester = semesterSpinBox->value();
            // Prevent semester 2 - if somehow it's still 2, change to 3
            if (semester == 2) {
                semester = 3;
            }
            int course = (semester - 1) / 2 + 1; // Calculate course from semester: 1-2 = course 1, 3-4 = course 2, etc.
            auto student = std::make_shared<Student>(
                name.toStdString(), surname.toStdString(), course,
                semester, averageGradeSpinBox->value(), isBudget);

            student->setMissedHours(missedHoursSpinBox->value());
            student->setHasSocialScholarship(socialScholarshipCheckBox->isChecked());

            int currentSem = semesterSpinBox->value();
            if (currentSem > 1) {
                // Generate random grades for previous semesters
                std::random_device rd;
                std::mt19937 gen(rd());
                // Grades will be in range 5.0 to 10.0 with some variation
                std::uniform_real_distribution<double> gradeDistribution(5.0, 10.0);
                
                for (int sem = 1; sem < currentSem; ++sem) {
                    double randomGrade = gradeDistribution(gen);
                    // Round to 2 decimal places
                    randomGrade = std::round(randomGrade * 100.0) / 100.0;
                    student->addPreviousGrade(sem, randomGrade);
                }
            }

            database.addStudent(student);

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

            QMessageBox::information(this, "Success", "Student added successfully!");
        } catch (const ValidationException& e) {
            QMessageBox::critical(this, "Validation Error", e.what());
        } catch (const ScholarshipException& e) {
            QMessageBox::critical(this, "Error", e.what());
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Unexpected Error", e.what());
        }
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
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Search Error", e.what());
    }
}

void MainWindow::calculateAllScholarships() {
    auto allStudents = database.getAllStudents();
    int count = 0;

    for (auto& student : allStudents) {
        student->recalculateScholarship();
        count++;
    }

    scholarshipsCalculated = true;
    scholarshipsNeedRecalculation = false;
    recalculationWarning->setVisible(false);

    if (studentTable->columnCount() == 9) {
        studentTable->insertColumn(9);
        studentTable->setHorizontalHeaderItem(9, new QTableWidgetItem("Scholarship (BYN)"));
        studentTable->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    }

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

    int currentColumnCount = studentTable->columnCount();

    // Block signals to prevent updateSelectionVisual from being called during item creation
    studentTable->blockSignals(true);
    studentTable->setRowCount(0);

    if (scholarshipsCalculated && currentColumnCount == 9) {
        studentTable->insertColumn(9);
        studentTable->setHorizontalHeaderItem(9, new QTableWidgetItem("Scholarship (BYN)"));

        studentTable->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);

        studentTable->horizontalHeader()->setStyleSheet(
            "QHeaderView::section {"
            "background-color: #2A2A2A;"
            "color: #EAEAEA;"
            "padding: 16px 14px;"
            "font-weight: 700;"
            "border: none;"
            "border-bottom: 2px solid #0d7377;"
            "font-size: 12px;"
            "min-height: 40px;"
            "}"
            "QHeaderView::section:first {"
            "background-color: transparent;"
            "color: #EAEAEA;"
            "border-bottom: none;"
            "}");

        studentTable->horizontalHeader()->setStretchLastSection(true);
    }

    int rowNum = 1;
    for (const auto& student : studentList) {
        if (!student) continue;

        int row = studentTable->rowCount();
        studentTable->insertRow(row);

        double avgGrade = student->getAverageGrade();
        double scholarship = student->getScholarship();

        QColor defaultTextColor(234, 234, 234); // #EAEAEA
        QFont itemFont;
        itemFont.setPointSize(11);

        QTableWidgetItem* numItem = new QTableWidgetItem(QString::number(rowNum++));
        numItem->setTextAlignment(Qt::AlignCenter);
        numItem->setBackground(QBrush(QColor(0, 0, 0, 0)));          
        numItem->setForeground(QBrush(QColor(180, 180, 180)));        
        numItem->setFlags(numItem->flags() & ~Qt::ItemIsSelectable);  
        numItem->setData(Qt::UserRole, QVariant());  
        numItem->setFont(itemFont);
        studentTable->setItem(row, 0, numItem);

        QTableWidgetItem* nameItem =
            new QTableWidgetItem(QString::fromStdString(student->getName()));
        nameItem->setForeground(QBrush(defaultTextColor));
        nameItem->setFont(itemFont);
        studentTable->setItem(row, 1, nameItem);

        QTableWidgetItem* surnameItem =
            new QTableWidgetItem(QString::fromStdString(student->getSurname()));
        surnameItem->setForeground(QBrush(defaultTextColor));
        surnameItem->setFont(itemFont);
        studentTable->setItem(row, 2, surnameItem);

        QTableWidgetItem* courseItem = new QTableWidgetItem(QString::number(student->getCourse()));
        courseItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        courseItem->setForeground(QBrush(defaultTextColor));
        courseItem->setFont(itemFont);
        studentTable->setItem(row, 3, courseItem);

        QTableWidgetItem* semesterItem =
            new QTableWidgetItem(QString::number(student->getSemester()));
        semesterItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        semesterItem->setForeground(QBrush(defaultTextColor));
        semesterItem->setFont(itemFont);
        studentTable->setItem(row, 4, semesterItem);

        QString funding = student->getIsBudget() ? "Budget" : "Paid";
        QTableWidgetItem* fundingItem = new QTableWidgetItem(funding);
        fundingItem->setForeground(QBrush(defaultTextColor));
        fundingItem->setFont(itemFont);
        studentTable->setItem(row, 5, fundingItem);

        QTableWidgetItem* gradeItem = new QTableWidgetItem(QString::number(avgGrade, 'f', 2));
        gradeItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        gradeItem->setForeground(QBrush(defaultTextColor));
        gradeItem->setFont(itemFont);
        studentTable->setItem(row, 6, gradeItem);

        QTableWidgetItem* missedItem =
            new QTableWidgetItem(QString::number(student->getMissedHours()));
        missedItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        // Highlight missed hours in red if >= 12
        if (student->getMissedHours() >= 12) {
            QColor redColor(255, 0, 0); // Red color
            missedItem->setData(Qt::UserRole + 10, true); // Mark as high missed hours
            missedItem->setData(Qt::UserRole + 11, redColor); // Store red color for restoration
            missedItem->setForeground(QBrush(redColor));
            QFont boldFont = itemFont;
            boldFont.setBold(true);
            missedItem->setFont(boldFont);
        } else {
            missedItem->setForeground(QBrush(defaultTextColor));
            missedItem->setFont(itemFont);
        }
        studentTable->setItem(row, 7, missedItem);

        QTableWidgetItem* socialItem =
            new QTableWidgetItem(student->getHasSocialScholarship() ? "Yes" : "No");
        socialItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        socialItem->setData(Qt::UserRole, 0); 
        socialItem->setForeground(QBrush(defaultTextColor));
        socialItem->setFont(itemFont);
        studentTable->setItem(row, 8, socialItem);

        if (scholarshipsCalculated) {
            QTableWidgetItem* scholarshipItem =
                new QTableWidgetItem(QString::number(scholarship, 'f', 2));
            scholarshipItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            scholarshipItem->setData(Qt::UserRole, 0);
            scholarshipItem->setData(Qt::UserRole + 2, scholarship);
            if (scholarship > 0) {
                scholarshipItem->setData(Qt::UserRole + 1, QColor(76, 175, 80));
                QFont boldFont = itemFont;
                boldFont.setBold(true);
                scholarshipItem->setFont(boldFont);
                scholarshipItem->setForeground(QBrush(QColor(76, 175, 80)));
            } else {
                scholarshipItem->setData(Qt::UserRole + 1, QColor(158, 158, 158));
                scholarshipItem->setFont(itemFont);
                scholarshipItem->setForeground(QBrush(QColor(158, 158, 158)));
            }
            studentTable->setItem(row, 9, scholarshipItem);
        }
    }

    if (studentTable->columnCount() == 10 && scholarshipsCalculated) {
        for (int i = 0; i < 9; ++i) {
            studentTable->resizeColumnToContents(i);
        }
        // Enable stretching for the last column (Scholarship) to fill remaining space
        studentTable->horizontalHeader()->setStretchLastSection(true);
    } else {
        studentTable->resizeColumnsToContents();
    }

    updateRowNumbers();
    
    // Unblock signals - this will trigger updateSelectionVisual if needed
    studentTable->blockSignals(false);
    
    // Explicitly apply red color to missed hours >= 12 after all items are created
    // This ensures red color is applied even if updateSelectionVisual was called
    for (int row = 0; row < studentTable->rowCount(); ++row) {
        QTableWidgetItem* missedItem = studentTable->item(row, 7);
        if (missedItem && missedItem->data(Qt::UserRole + 10).toBool()) {
            QVariant colorVar = missedItem->data(Qt::UserRole + 11);
            QColor redColor(255, 0, 0);
            if (colorVar.isValid()) {
                redColor = colorVar.value<QColor>();
            }
            // Force red color
            missedItem->setForeground(QBrush(redColor));
            // Ensure font is bold
            QFont font = missedItem->font();
            font.setBold(true);
            missedItem->setFont(font);
        }
    }
}

void MainWindow::updateStatistics() {
    auto allStudents = database.getAllStudents();
    size_t total = allStudents.size();
    size_t budget = 0;
    size_t paid = 0;
    double totalScholarship = 0.0;

    for (const auto& student : allStudents) {
        if (student->getIsBudget()) {
            budget++;
        } else {
            paid++;
        }
        totalScholarship += student->getScholarship();
    }

    totalStudentsLabel->setText(QString("Total Students: %1").arg(total));
    budgetStudentsLabel->setText(QString("Budget Students: %1").arg(budget));
    paidStudentsLabel->setText(QString("Paid Students: %1").arg(paid));
    totalScholarshipLabel->setText(
        QString("Total Scholarships: %1 BYN").arg(totalScholarship, 0, 'f', 2));

    // Update semester statistics table
    if (semesterStatsTable) {
        updateSemesterStatisticsTable();
    }
}

int MainWindow::getAdmissionYearFromSemester(int semester) const {
    // Admission year mapping:
    // Semesters 1-2 -> 2025
    // Semesters 3-4 -> 2024
    // Semesters 5-6 -> 2023
    // Semesters 7-8 -> 2022
    return 2026 - ((semester + 1) / 2);
}

int MainWindow::getYearForSemester(int semester) const {
    // Year mapping:
    // Semesters 1-2 -> 2025
    // Semesters 3-4 -> 2024
    // Semesters 5-6 -> 2023
    // Semesters 7-8 -> 2022
    return 2026 - ((semester + 1) / 2);
}

int MainWindow::getYearForSemester(int semester, int admissionYear) const {
    // Calculate the year for a semester based on admission year
    // Semester 1 (Winter) = admissionYear
    // Semester 2 (Summer) = admissionYear + 1 (but semester 2 is forbidden)
    // Semester 3 (Winter) = admissionYear + 1
    // Semester 4 (Summer) = admissionYear + 1
    // Semester 5 (Winter) = admissionYear + 2
    // Semester 6 (Summer) = admissionYear + 2
    // Semester 7 (Winter) = admissionYear + 3
    // Semester 8 (Summer) = admissionYear + 3
    
    // Formula: year = admissionYear + floor((semester - 1) / 2)
    // But for even semesters (summer), it's the same year as the next odd semester
    // So: year = admissionYear + (semester - 1) / 2 for odd, admissionYear + semester / 2 for even
    
    if (semester % 2 == 1) {
        // Odd semester (Winter): semester 1 = year 0, semester 3 = year 1, etc.
        return admissionYear + (semester - 1) / 2;
    } else {
        // Even semester (Summer): semester 2 = year 1, semester 4 = year 1, etc.
        return admissionYear + semester / 2;
    }
}

QString MainWindow::getSessionTypeForSemester(int semester) const {
    // Odd semesters (1, 3, 5, 7) = Winter session (зимняя сессия)
    // Even semesters (2, 4, 6, 8) = Summer session (летняя сессия)
    return (semester % 2 == 1) ? "Winter" : "Summer";
}

void MainWindow::updateSemesterStatisticsTable() {
    if (!semesterStatsTable) return;

    auto allStudents = database.getAllStudents();

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

    for (const auto& student : allStudents) {
        if (!student) continue;
        bool isBudget = student->getIsBudget();
        int currentSem = student->getSemester();
        const auto& history = student->getPreviousSemesterGrades();

        // Collect all semesters this student has been through (current + history)
        std::set<int> allSemesters;
        allSemesters.insert(currentSem);
        for (const auto& entry : history) {
            allSemesters.insert(entry.first);
        }

        // For each year, check if student was studying in that year
        for (int year = 2022; year <= 2025; ++year) {
            // Check all semesters that belong to this year
            std::vector<int> yearSemesters;
            for (int sem = 1; sem <= 8; ++sem) {
                if (getYearForSemester(sem) == year) {
                    yearSemesters.push_back(sem);
                }
            }

            // Check if student was on any semester in this year
            bool wasInYear = false;
            for (int sem : yearSemesters) {
                if (allSemesters.find(sem) != allSemesters.end()) {
                    wasInYear = true;
                    break;
                }
            }

            if (!wasInYear) continue;

            auto& stats = yearStats[year];

            // Winter: count student if they were on ANY semester in this year
            stats.winterCount += 1;
            
            // Calculate scholarship for winter (use current semester if in this year, otherwise use first semester from history)
            double winterScholarship = 0.0;
            if (isBudget) {
                if (getYearForSemester(currentSem) == year) {
                    // Current semester is in this year
                    if (student->getMissedHours() < 12) {
                        winterScholarship = student->getScholarship();
                    }
                } else {
                    // Find first semester from history in this year
                    for (int sem : yearSemesters) {
                        auto it = history.find(sem);
                        if (it != history.end()) {
                            winterScholarship = ScholarshipCalculator::calculateScholarship(it->second);
                            break;
                        }
                    }
                }
            }
            stats.winterTotal += winterScholarship;

            // Summer: count student only if they were on an even semester (Summer session) in this year
            bool wasOnSummerSemester = false;
            for (int sem : yearSemesters) {
                if (sem % 2 == 0 && allSemesters.find(sem) != allSemesters.end()) {
                    wasOnSummerSemester = true;
                    break;
                }
            }

            if (wasOnSummerSemester) {
                stats.summerCount += 1;
                
                // Calculate scholarship for summer
                double summerScholarship = 0.0;
                if (isBudget) {
                    // Find even semester (Summer) from this year
                    for (int sem : yearSemesters) {
                        if (sem % 2 == 0) {
                            if (sem == currentSem && getYearForSemester(currentSem) == year) {
                                // Current semester is summer in this year
                                if (student->getMissedHours() < 12) {
                                    summerScholarship = student->getScholarship();
                                }
                                break;
                            } else {
                                auto it = history.find(sem);
                                if (it != history.end()) {
                                    summerScholarship = ScholarshipCalculator::calculateScholarship(it->second);
                                    break;
                                }
                            }
                        }
                    }
                }
                stats.summerTotal += summerScholarship;
            }
        }
    }

    semesterStatsTable->setRowCount(0);
    QColor defaultTextColor(234, 234, 234);
    QFont itemFont;
    itemFont.setPointSize(11);

    for (int year = 2025; year >= 2022; --year) {
        YearStats stats{};
        auto it = yearStats.find(year);
        if (it != yearStats.end()) {
            stats = it->second;
        }

        // Winter row
        int winterRow = semesterStatsTable->rowCount();
        semesterStatsTable->insertRow(winterRow);

        QString winterLabel = QString("%1 (Winter)").arg(year);
        QTableWidgetItem* winterItem = new QTableWidgetItem(winterLabel);
        winterItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        winterItem->setForeground(QBrush(defaultTextColor));
        winterItem->setFont(itemFont);
        winterItem->setFlags(winterItem->flags() & ~Qt::ItemIsSelectable);
        semesterStatsTable->setItem(winterRow, 0, winterItem);

        QTableWidgetItem* winterCountItem = new QTableWidgetItem(QString::number(stats.winterCount));
        winterCountItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        winterCountItem->setForeground(QBrush(defaultTextColor));
        winterCountItem->setFont(itemFont);
        winterCountItem->setFlags(winterCountItem->flags() & ~Qt::ItemIsSelectable);
        semesterStatsTable->setItem(winterRow, 1, winterCountItem);

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
        semesterStatsTable->setItem(winterRow, 2, winterTotalItem);

        // Summer row

        int summerRow = semesterStatsTable->rowCount();
        semesterStatsTable->insertRow(summerRow);

        QString summerLabel = QString("%1 (Summer)").arg(year);
        QTableWidgetItem* summerItem = new QTableWidgetItem(summerLabel);
        summerItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        summerItem->setForeground(QBrush(defaultTextColor));
        summerItem->setFont(itemFont);
        summerItem->setFlags(summerItem->flags() & ~Qt::ItemIsSelectable);
        semesterStatsTable->setItem(summerRow, 0, summerItem);

        QTableWidgetItem* summerCountItem =
            new QTableWidgetItem(QString::number(stats.summerCount));
        summerCountItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        summerCountItem->setForeground(QBrush(defaultTextColor));
        summerCountItem->setFont(itemFont);
        summerCountItem->setFlags(summerCountItem->flags() & ~Qt::ItemIsSelectable);
        semesterStatsTable->setItem(summerRow, 1, summerCountItem);

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
        semesterStatsTable->setItem(summerRow, 2, summerTotalItem);
    }

    semesterStatsTable->resizeColumnsToContents();
}

void MainWindow::editSelectedStudent() {
    int row = studentTable->currentRow();
    if (row < 0 || row >= static_cast<int>(currentView.size())) {
        QMessageBox::warning(this, "Error", "Please select a student to edit.");
        return;
    }

    auto student = currentView[row];
    if (!student) return;

    QDialog dlg(this);
    dlg.setWindowTitle("Edit Student");
    dlg.setMinimumWidth(400);
    dlg.setStyleSheet(
        "QDialog { background-color: #2d2d2d; color: #ffffff; }"
        "QLabel { "
        "color: #e0e0e0; "
        "font-size: 12px; "
        "background: none; "
        "background-color: rgba(0,0,0,0); "
        "border: 0px; "
        "padding: 0px; "
        "}"
        "QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox { "
        "background-color: #1e1e1e; "
        "color: #EAEAEA; "
        "border: 2px solid #0d7377; "
        "border-radius: 4px; "
        "padding: 8px; "
        "}"
        "QLineEdit::placeholder, QSpinBox::placeholder, QDoubleSpinBox::placeholder { "
        "color: #888888; "
        "}"
        "QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus { "
        "border-color: #14a085; "
        "background-color: #252525; "
        "}"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { "
        "background-color: #1e1e1e; "
        "color: #EAEAEA; "
        "selection-background-color: #0d7377; "
        "}");
    QFormLayout* form = new QFormLayout(&dlg);
    form->setSpacing(15);
    form->setContentsMargins(20, 20, 20, 20);

    QLineEdit* nameField = new QLineEdit(QString::fromStdString(student->getName()), &dlg);
    QLineEdit* surnameField = new QLineEdit(QString::fromStdString(student->getSurname()), &dlg);
    QSpinBox* semesterField = new QSpinBox(&dlg);
    semesterField->setRange(1, 8);
    int initialSemester = student->getSemester();
    // If student is on semester 2, set to 3 instead
    if (initialSemester == 2) {
        initialSemester = 3;
    }
    semesterField->setValue(initialSemester);
    // Prevent selecting semester 2
    connect(semesterField, QOverload<int>::of(&QSpinBox::valueChanged), [semesterField](int value) {
        if (value == 2) {
            semesterField->setValue(3);
        }
    });
    // Course is automatically calculated from semester, so we don't need courseField

    QDoubleSpinBox* avgField = new QDoubleSpinBox(&dlg);
    avgField->setRange(0.0, 10.0);
    avgField->setDecimals(2);
    avgField->setSingleStep(0.1);
    avgField->setValue(student->getAverageGrade());

    QComboBox* fundingField = new QComboBox(&dlg);
    fundingField->addItem("Budget");
    fundingField->addItem("Paid");
    fundingField->setCurrentIndex(student->getIsBudget() ? 0 : 1);

    QSpinBox* missedHoursField = new QSpinBox(&dlg);
    missedHoursField->setRange(0, 100);
    missedHoursField->setValue(student->getMissedHours());

    QCheckBox* socialCheckBox = new QCheckBox(&dlg);
    socialCheckBox->setText("Social Scholarship");
    socialCheckBox->setChecked(student->getHasSocialScholarship());
    socialCheckBox->setStyleSheet("color: #e0e0e0;");

    form->addRow("Name:", nameField);
    form->addRow("Surname:", surnameField);
    form->addRow("Semester:", semesterField);
    form->addRow("Average Grade:", avgField);
    form->addRow("Funding Type:", fundingField);
    form->addRow("Missed Hours:", missedHoursField);
    form->addRow("", socialCheckBox);

    QDialogButtonBox* bb =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    bb->button(QDialogButtonBox::Ok)->setText("Save");
    bb->button(QDialogButtonBox::Cancel)->setText("Cancel");
    bb->button(QDialogButtonBox::Ok)
        ->setStyleSheet(
            "QPushButton { background-color: #4CAF50; color: white; padding: 8px 20px; "
            "border-radius: 4px; font-weight: bold; }"
            "QPushButton:hover { background-color: #45a049; }");
    bb->button(QDialogButtonBox::Cancel)
        ->setStyleSheet(
            "QPushButton { background-color: #f44336; color: white; padding: 8px 20px; "
            "border-radius: 4px; font-weight: bold; }"
            "QPushButton:hover { background-color: #da190b; }");
    form->addRow(bb);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        if (nameField->text().trimmed().isEmpty() || surnameField->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Error", "Name and surname cannot be empty.");
            return;
        }

        try {
            int oldSemester = student->getSemester();
            double oldGrade = student->getAverageGrade();
            int newSemester = semesterField->value();
            
            // Prevent semester 2 - if somehow it's still 2, change to 3
            if (newSemester == 2) {
                newSemester = 3;
            }

            if (oldSemester != newSemester && oldSemester > 0) {
                student->addPreviousGrade(oldSemester, oldGrade);
            }

            student->setName(nameField->text().trimmed().toStdString());
            student->setSurname(surnameField->text().trimmed().toStdString());
            // Course is automatically calculated when setting semester
            student->setSemester(newSemester);
            student->setAverageGrade(avgField->value());
            student->setIsBudget(fundingField->currentText() == "Budget");
            student->setMissedHours(missedHoursField->value());
            student->setHasSocialScholarship(socialCheckBox->isChecked());

            // Ensure all previous semesters have grades (generate random if missing)
            const auto& existingHistory = student->getPreviousSemesterGrades();
            if (newSemester > 1) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<double> gradeDistribution(5.0, 10.0);
                
                for (int sem = 1; sem < newSemester; ++sem) {
                    // If this semester doesn't have a grade, generate a random one
                    if (existingHistory.find(sem) == existingHistory.end()) {
                        double randomGrade = gradeDistribution(gen);
                        randomGrade = std::round(randomGrade * 100.0) / 100.0;
                        student->addPreviousGrade(sem, randomGrade);
                    }
                }
            }

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

    QDialog historyDialog(this);
    historyDialog.setWindowTitle(QString("History: %1 %2")
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

    // Current Semester Information Table
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
    currentInfoTable->horizontalHeader()->setStretchLastSection(true);
    currentInfoTable->setShowGrid(false);
    
    currentInfoTable->setStyleSheet(
        "QTableWidget {"
        "background-color: #1B1B1B;"
        "alternate-background-color: #1E1E1E;"
        "border: 2px solid #0d7377;"
        "border-radius: 10px;"
        "}"
        "QTableWidget::item {"
        "padding: 12px;"
        "border: none;"
        "color: #EAEAEA;"
        "font-size: 13px;"
        "}"
        "QTableWidget::item:hover {"
        "background-color: #2d2d2d;"        
        "}"
        "QHeaderView::section {"
        "background-color: #0d7377;"
        "color: #ffffff;"
        "padding: 12px;"
        "font-weight: bold;"
        "border: none;"
        "border-bottom: 2px solid #14a085;"
        "font-size: 13px;"
        "}"
        "QHeaderView::section:first {"
        "border-top-left-radius: 8px;"
        "}"
        "QHeaderView::section:last {"
        "border-top-right-radius: 8px;"
        "}");
    
    QColor defaultTextColor(234, 234, 234);
    
    // Get current year to calculate admission year correctly for history
    int currentSystemYear = QDate::currentDate().year();
    
    // Calculate admission year based on the minimum semester (current or from history)
    // This ensures correct year calculation even when semester is changed
    // We calculate backwards from current year to ensure history shows past years only
    int semester = student->getSemester();
    int minSemester = semester;
    const auto& previousGrades = student->getPreviousSemesterGrades();
    for (const auto& pair : previousGrades) {
        if (pair.first < minSemester) {
            minSemester = pair.first;
        }
    }
    
    // Find the maximum semester (current or from history) to ensure all history is in the past
    int maxSemester = semester;
    for (const auto& pair : previousGrades) {
        if (pair.first > maxSemester) {
            maxSemester = pair.first;
        }
    }
    
    // Calculate admission year based on MAXIMUM semester to ensure all semesters (including history) are in the past
    // For odd semesters (Winter): years = (maxSemester - 1) / 2
    // Example: maxSemester 5 (winter) = currentYear - (5-1)/2 = currentYear - 2
    // This ensures all history entries are in the past, even if student moved to a lower semester
    // We use maxSemester to calculate admission year so that the highest semester in history doesn't exceed current year
    int yearsFromAdmission = (maxSemester - 1) / 2;
    int admissionYear = currentSystemYear - yearsFromAdmission;
    
    // Ensure admission year is not in the future (shouldn't happen, but safety check)
    if (admissionYear > currentSystemYear) {
        admissionYear = currentSystemYear;
    }
    
    int currentYear = getYearForSemester(semester, admissionYear);
    
    // Show current semester with year and session type
    QString currentSessionType = getSessionTypeForSemester(student->getSemester());
    QString currentSemesterInfo = QString("%1 (%2 %3)").arg(student->getSemester()).arg(currentYear).arg(currentSessionType);
    
    QStringList properties = {"Admission Year", "Course", "Semester / Year / Session", "Average Grade", "Funding Type", 
                              "Missed Hours", "Social Scholarship", "Scholarship"};
    QStringList values = {
        QString::number(admissionYear),
        QString::number(student->getCourse()),
        currentSemesterInfo,
        QString::number(student->getAverageGrade(), 'f', 2),
        student->getIsBudget() ? "Budget" : "Paid",
        QString::number(student->getMissedHours()),
        student->getHasSocialScholarship() ? "Yes" : "No",
        QString::number(student->getScholarship(), 'f', 2) + " BYN"
    };
    
    // Update table row count for 8 rows (added Admission Year)
    currentInfoTable->setRowCount(8);
    
    for (int i = 0; i < 8; ++i) {
        QTableWidgetItem* propertyItem = new QTableWidgetItem(properties[i]);
        propertyItem->setForeground(QBrush(defaultTextColor));
        propertyItem->setFlags(propertyItem->flags() & ~Qt::ItemIsSelectable);
        currentInfoTable->setItem(i, 0, propertyItem);
        
        QTableWidgetItem* valueItem = new QTableWidgetItem(values[i]);
        valueItem->setForeground(QBrush(defaultTextColor));
        if (i == 7 && student->getScholarship() > 0) {
            valueItem->setForeground(QBrush(QColor(76, 175, 80)));
            valueItem->setFont(QFont("", -1, QFont::Bold));
        }
        valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsSelectable);
        currentInfoTable->setItem(i, 1, valueItem);
    }
    
    currentInfoTable->resizeColumnsToContents();
    currentInfoTable->setMaximumHeight(280);
    layout->addWidget(currentInfoTable);

    // Previous Semesters History Table
    QLabel* historyLabel = new QLabel("Previous Semesters History:", &historyDialog);
    historyLabel->setStyleSheet(
        "font-weight: bold; font-size: 14px; color: #14a085; margin-top: 10px; background: none; "
        "background-color: rgba(0,0,0,0); border: 0px; padding: 0px;");
    layout->addWidget(historyLabel);

    QTableWidget* historyTable = new QTableWidget(&historyDialog);
    historyTable->setColumnCount(3);
    historyTable->setHorizontalHeaderLabels(QStringList() << "Semester / Year / Session" << "Average Grade" << "Scholarship (BYN)");
    historyTable->verticalHeader()->setVisible(false);
    historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    historyTable->setAlternatingRowColors(true);
    historyTable->horizontalHeader()->setStretchLastSection(true);
    historyTable->setShowGrid(false);
    
    historyTable->setStyleSheet(
        "QTableWidget {"
        "background-color: #1B1B1B;"
        "alternate-background-color: #1E1E1E;"
        "border: 2px solid #0d7377;"
        "border-radius: 10px;"
        "}"
        "QTableWidget::item {"
        "padding: 12px;"
        "border: none;"
        "color: #EAEAEA;"
        "font-size: 13px;"
        "}"
        "QTableWidget::item:hover {"
        "background-color: #2d2d2d;"
        "}"
        "QHeaderView::section {"
        "background-color: #0d7377;"
        "color: #ffffff;"
        "padding: 12px;"
        "font-weight: bold;"
        "border: none;"
        "border-bottom: 2px solid #14a085;"
        "font-size: 13px;"
        "}"
        "QHeaderView::section:first {"
        "border-top-left-radius: 8px;"
        "}"
        "QHeaderView::section:last {"
        "border-top-right-radius: 8px;"
        "}");

    // Use the admission year and previousGrades already calculated above
    if (previousGrades.empty()) {
        historyTable->setRowCount(1);
        QString noDataText = "No previous semester data available. This student is on their first semester.";
        QTableWidgetItem* noDataItem = new QTableWidgetItem(noDataText);
        noDataItem->setForeground(QBrush(defaultTextColor));
        noDataItem->setFlags(noDataItem->flags() & ~Qt::ItemIsSelectable);
        noDataItem->setTextAlignment(Qt::AlignCenter);
        historyTable->setItem(0, 0, noDataItem);
        // Fill other columns with empty items for span to work
        for (int col = 1; col < 3; ++col) {
            QTableWidgetItem* emptyItem = new QTableWidgetItem("");
            emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsSelectable);
            historyTable->setItem(0, col, emptyItem);
        }
        historyTable->setSpan(0, 0, 1, 3);
        historyTable->horizontalHeader()->setStretchLastSection(true);
    } else {
        std::vector<std::pair<int, double>> sortedHistory;
        for (const auto& pair : previousGrades) {
            sortedHistory.push_back(pair);
        }
        std::sort(sortedHistory.begin(), sortedHistory.end());

        historyTable->setRowCount(sortedHistory.size());
        
        for (size_t i = 0; i < sortedHistory.size(); ++i) {
            int sem = sortedHistory[i].first;
            double grade = sortedHistory[i].second;

            double prevScholarship = 0.0;
            if (student->getIsBudget()) {
                prevScholarship = ScholarshipCalculator::calculateScholarship(grade);
            }

            // Semester column (0) - show year and session type
            // Use the admission year calculated from minimum semester
            int year = getYearForSemester(sem, admissionYear);
            QString sessionType = getSessionTypeForSemester(sem);
            QString semesterText = QString("%1 (%2 %3)").arg(sem).arg(year).arg(sessionType);
            QTableWidgetItem* semesterItem = new QTableWidgetItem(semesterText);
            semesterItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            semesterItem->setForeground(QBrush(defaultTextColor));
            semesterItem->setFlags(semesterItem->flags() & ~Qt::ItemIsSelectable);
            historyTable->setItem(i, 0, semesterItem);

            // Average Grade column (1)
            QTableWidgetItem* gradeItem = new QTableWidgetItem(QString::number(grade, 'f', 2));
            gradeItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            gradeItem->setForeground(QBrush(defaultTextColor));
            gradeItem->setFlags(gradeItem->flags() & ~Qt::ItemIsSelectable);
            historyTable->setItem(i, 1, gradeItem);

            // Scholarship column (2)
            QTableWidgetItem* scholarshipItem = new QTableWidgetItem(QString::number(prevScholarship, 'f', 2));
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
    connect(closeButton, &QPushButton::clicked, &historyDialog, &QDialog::accept);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    historyDialog.exec();
}

void MainWindow::fillMissingHistoryGrades() {
    auto allStudents = database.getAllStudents();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> gradeDistribution(5.0, 10.0);
    
    for (auto& student : allStudents) {
        int currentSem = student->getSemester();
        if (currentSem > 1) {
            const auto& existingHistory = student->getPreviousSemesterGrades();
            
            for (int sem = 1; sem < currentSem; ++sem) {
                // If this semester doesn't have a grade, generate a random one
                if (existingHistory.find(sem) == existingHistory.end()) {
                    double randomGrade = gradeDistribution(gen);
                    randomGrade = std::round(randomGrade * 100.0) / 100.0;
                    student->addPreviousGrade(sem, randomGrade);
                }
            }
        }
    }
}

void MainWindow::updateRowNumbers() {
    for (int row = 0; row < studentTable->rowCount(); ++row) {
        QTableWidgetItem* numItem = studentTable->item(row, 0);
        if (numItem) {
            numItem->setText(QString::number(row + 1));
        } else {
            numItem = new QTableWidgetItem(QString::number(row + 1));
            numItem->setTextAlignment(Qt::AlignCenter);
            numItem->setBackground(QBrush(QColor(0, 0, 0, 0)));           
            numItem->setForeground(QBrush(QColor(180, 180, 180)));        
            numItem->setFlags(numItem->flags() & ~Qt::ItemIsSelectable);  
            studentTable->setItem(row, 0, numItem);
        }
    }
}
