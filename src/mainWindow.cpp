#include "mainWindow.h"
#include "exceptions.h"
#include <QApplication>
#include <QStringList>
#include <QBrush>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFont>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <map>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupUI();
    
    database.setFilename("students.txt");
    
    try {
        database.loadFromFile();
    } catch (const FileIOException& e) {
        QMessageBox::warning(this, "File Error", 
            QString("Could not load file: %1\nStarting with empty database.").arg(e.what()));
    } catch (const ParseException& e) {
        QMessageBox::warning(this, "Validation Error",
            QString("Some students were skipped due to invalid data:\n\n%1\n\nPlease check the file and fix the errors.").arg(e.what()));
    }
    
    showAllStudents();
    updateStatistics();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    setWindowTitle("Scholarship Management System");
    setMinimumSize(1400, 800);
    resize(1400, 800);
    
    // Dark theme
    setStyleSheet(
        "QMainWindow { background-color: #1e1e1e; }"
        "QWidget { background-color: #1e1e1e; color: #ffffff; }"
        "QTabWidget::pane { border: 1px solid #3d3d3d; background-color: #2d2d2d; }"
        "QTabBar::tab { background-color: #2d2d2d; color: #ffffff; padding: 10px 20px; margin-right: 2px; border-top-left-radius: 5px; border-top-right-radius: 5px; }"
        "QTabBar::tab:selected { background-color: #0d7377; color: #ffffff; }"
        "QTabBar::tab:hover { background-color: #14a085; }"
        "QLabel { color: #ffffff; }"
    );

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
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // Top panel with search and buttons
    QHBoxLayout* topLayout = new QHBoxLayout();
    
    // Search
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
        "}"
    );
    
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
        "QPushButton:pressed { background-color: #0a5d61; }"
    );
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::searchStudent);
    connect(searchEdit, &QLineEdit::returnPressed, this, &MainWindow::searchStudent);
    
    topLayout->addWidget(searchLabel);
    topLayout->addWidget(searchEdit);
    topLayout->addWidget(searchButton);
    topLayout->addStretch();
    
    // Add student button
    addStudentButton = new QPushButton("Add Student", this);
    addStudentButton->setStyleSheet(
        "QPushButton {"
        "background-color: #0d7377;"
        "color: white;"
        "padding: 12px 30px;"
        "border-radius: 8px;"
        "font-weight: bold;"
        "font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #14a085; }"
        "QPushButton:pressed { background-color: #0a5d61; }"
    );
    connect(addStudentButton, &QPushButton::clicked, this, &MainWindow::addStudent);
    topLayout->addWidget(addStudentButton);
    
    mainLayout->addLayout(topLayout);

    // Warning message
    recalculationWarning = new QLabel("WARNING: Scholarship data may be outdated. Please recalculate!", this);
    recalculationWarning->setStyleSheet(
        "QLabel {"
        "background-color: #FF6B35;"
        "color: white;"
        "padding: 10px;"
        "border-radius: 5px;"
        "font-weight: bold;"
        "font-size: 13px;"
        "}"
    );
    recalculationWarning->setAlignment(Qt::AlignCenter);
    recalculationWarning->setVisible(false);
    mainLayout->addWidget(recalculationWarning);
    
    // Calculate scholarships button
    QHBoxLayout* calcLayout = new QHBoxLayout();
    calculateButton = new QPushButton("CALCULATE SCHOLARSHIPS", this);
    calculateButton->setStyleSheet(
        "QPushButton {"
        "background-color: #0d7377;"
        "color: white;"
        "padding: 12px 30px;"
        "border-radius: 8px;"
        "font-weight: bold;"
        "font-size: 14px;"
        "}"
        "QPushButton:hover { background-color: #14a085; }"
        "QPushButton:pressed { background-color: #0a5d61; }"
    );
    connect(calculateButton, &QPushButton::clicked, this, &MainWindow::calculateAllScholarships);
    calcLayout->addStretch();
    calcLayout->addWidget(calculateButton);
    calcLayout->addStretch();
    mainLayout->addLayout(calcLayout);

    // Student table
    createStudentTable();
    mainLayout->addWidget(studentTable, 1);

    // Action buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    editButton = new QPushButton("Edit Student", this);
    editButton->setStyleSheet(
        "QPushButton {"
        "background-color: #0d7377;"
        "color: white;"
        "padding: 10px 20px;"
        "border-radius: 5px;"
        "font-weight: bold;"
        "font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #14a085; }"
        "QPushButton:pressed { background-color: #0a5d61; }"
        "QPushButton:disabled { background-color: #3d3d3d; color: #666; }"
    );
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editSelectedStudent);
    
    deleteButton = new QPushButton("Delete Student", this);
    deleteButton->setStyleSheet(
        "QPushButton {"
        "background-color: #0d7377;"
        "color: white;"
        "padding: 10px 20px;"
        "border-radius: 5px;"
        "font-weight: bold;"
        "font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #14a085; }"
        "QPushButton:pressed { background-color: #0a5d61; }"
        "QPushButton:disabled { background-color: #3d3d3d; color: #666; }"
    );
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedStudent);
    
    changeFundingButton = new QPushButton("Change Funding Type", this);
    changeFundingButton->setStyleSheet(
        "QPushButton {"
        "background-color: #0d7377;"
        "color: white;"
        "padding: 10px 20px;"
        "border-radius: 5px;"
        "font-weight: bold;"
        "font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #14a085; }"
        "QPushButton:pressed { background-color: #0a5d61; }"
        "QPushButton:disabled { background-color: #3d3d3d; color: #666; }"
    );
    connect(changeFundingButton, &QPushButton::clicked, this, &MainWindow::changeFundingType);
    
    viewHistoryButton = new QPushButton("View History", this);
    viewHistoryButton->setStyleSheet(
        "QPushButton {"
        "background-color: #0d7377;"
        "color: white;"
        "padding: 10px 20px;"
        "border-radius: 5px;"
        "font-weight: bold;"
        "font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #14a085; }"
        "QPushButton:pressed { background-color: #0a5d61; }"
        "QPushButton:disabled { background-color: #3d3d3d; color: #666; }"
    );
    connect(viewHistoryButton, &QPushButton::clicked, this, &MainWindow::showStudentHistory);
    
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(changeFundingButton);
    buttonLayout->addWidget(viewHistoryButton);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
}

void MainWindow::createStatisticsTab(QWidget* tab) {
    QVBoxLayout* mainLayout = new QVBoxLayout(tab);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Statistics
    QGroupBox* statsGroup = new QGroupBox("Statistics", this);
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
        "color: #ffffff;"
        "}"
    );
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

    // Detailed statistics
    statisticsText = new QTextEdit(this);
    statisticsText->setReadOnly(true);
    statisticsText->setStyleSheet(
        "QTextEdit {"
        "border: 2px solid #3d3d3d;"
        "border-radius: 5px;"
        "padding: 10px;"
        "font-size: 12px;"
        "background-color: #2d2d2d;"
        "color: #ffffff;"
        "}"
    );
    mainLayout->addWidget(statisticsText, 1);
}

void MainWindow::createStudentTable() {
    studentTable = new QTableWidget(this);
    // Start with 9 columns (with numbering column, without scholarship column)
    studentTable->setColumnCount(9);
    QStringList headers = {"#", "Name", "Surname", "Course", "Semester", "Funding Type",
                          "Average Grade", "Missed Hours", "Social"};
    studentTable->setHorizontalHeaderLabels(headers);
    
    // Hide vertical row numbers (right side)
    studentTable->verticalHeader()->setVisible(false);
    
    studentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    studentTable->setSelectionMode(QAbstractItemView::SingleSelection);
    studentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    studentTable->setAlternatingRowColors(true);
    studentTable->horizontalHeader()->setStretchLastSection(true);
    studentTable->setSortingEnabled(true);
    studentTable->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "background-color: #2A2A2A;"
        "color: #EAEAEA;"
        "padding: 12px;"
        "font-weight: 600;"
        "border: none;"
        "border-bottom: 2px solid #0d7377;"
        "font-size: 12px;"
        "}"
        "QHeaderView::section:first {"
        "background-color: transparent;"
        "color: #EAEAEA;"
        "border-bottom: none;"
        "}"
    );
    studentTable->setStyleSheet(
        "QTableWidget {"
        "gridline-color: #2d2d2d;"
        "background-color: #1B1B1B;"
        "alternate-background-color: #1E1E1E;"
        "border: 2px solid #3d3d3d;"
        "border-radius: 5px;"
        "color: #EAEAEA;"
        "}"
        "QTableWidget::item {"
        "color: #EAEAEA;"
        "padding: 8px;"
        "border: none;"
        "}"
        "QTableWidget::item:selected {"
        "background-color: #0d7377;"
        "color: #ffffff;"
        "}"
        "QTableWidget::item:alternate {"
        "background-color: #1E1E1E;"
        "}"
    );
    
    connect(studentTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        bool hasSelection = studentTable->currentRow() >= 0;
        editButton->setEnabled(hasSelection);
        deleteButton->setEnabled(hasSelection);
        changeFundingButton->setEnabled(hasSelection);
        viewHistoryButton->setEnabled(hasSelection);
    });
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
        "}"
    );
    
    QFormLayout* form = new QFormLayout(&dialog);
    form->setSpacing(15);
    form->setContentsMargins(20, 20, 20, 20);
    
    nameEdit = new QLineEdit(&dialog);
    nameEdit->setPlaceholderText("Enter name");
    
    surnameEdit = new QLineEdit(&dialog);
    surnameEdit->setPlaceholderText("Enter surname");
    
    courseSpinBox = new QSpinBox(&dialog);
    courseSpinBox->setRange(1, 4);
    courseSpinBox->setValue(1);
    
    semesterSpinBox = new QSpinBox(&dialog);
    semesterSpinBox->setRange(1, 8);
    semesterSpinBox->setValue(1);
    
    connect(courseSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onCourseChanged);
    onCourseChanged(1);
    
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
    form->addRow("Course:", courseSpinBox);
    form->addRow("Semester:", semesterSpinBox);
    form->addRow("Average Grade:", averageGradeSpinBox);
    form->addRow("Funding Type:", fundingCombo);
    form->addRow("Missed Hours:", missedHoursSpinBox);
    form->addRow("", socialScholarshipCheckBox);
    
    QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    bb->button(QDialogButtonBox::Ok)->setText("Add");
    bb->button(QDialogButtonBox::Cancel)->setText("Cancel");
    bb->button(QDialogButtonBox::Ok)->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; padding: 8px 20px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #45a049; }"
    );
    bb->button(QDialogButtonBox::Cancel)->setStyleSheet(
        "QPushButton { background-color: #f44336; color: white; padding: 8px 20px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #da190b; }"
    );
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
            auto student = std::make_shared<Student>(
                name.toStdString(), 
                surname.toStdString(),
                courseSpinBox->value(), 
                semesterSpinBox->value(), 
                averageGradeSpinBox->value(), 
                isBudget
            );
            
            student->setMissedHours(missedHoursSpinBox->value());
            student->setHasSocialScholarship(socialScholarshipCheckBox->isChecked());
            
            // Save previous semester history (if not first semester)
            int currentSem = semesterSpinBox->value();
            if (currentSem > 1) {
                for (int sem = 1; sem < currentSem; ++sem) {
                    student->addPreviousGrade(sem, averageGradeSpinBox->value() - 0.2 * (currentSem - sem));
                }
            }
            
            database.addStudent(student);
            
            // Show warning if scholarships were already calculated
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
        showAllStudents(); // Show all students after failed search
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
    
    // Add scholarship column if not present (9 columns = without scholarship, 10 = with scholarship)
    if (studentTable->columnCount() == 9) {
        studentTable->insertColumn(9);
        studentTable->setHorizontalHeaderItem(9, new QTableWidgetItem("Scholarship (BYN)"));
    }
    
    database.saveToFile();
    showAllStudents();
    updateStatistics();
    
    QMessageBox::information(this, "Success", 
        QString("Successfully calculated scholarships for %1 students.").arg(count));
}

void MainWindow::onCourseChanged(int course) {
    updateSemesterRange(course);
}

void MainWindow::showAllStudents() {
    std::vector<std::shared_ptr<Student>> allStudents = database.getAllStudents();
    updateStudentTable(allStudents);
}

void MainWindow::updateStudentTable(const std::vector<std::shared_ptr<Student>>& studentList) {
    currentView = studentList;
    
    // Save current column count before clearing rows
    int currentColumnCount = studentTable->columnCount();
    
    studentTable->setRowCount(0);
    
    // Only add scholarship column once when first calculated
    if (scholarshipsCalculated && currentColumnCount == 9) {
        studentTable->insertColumn(9);
        studentTable->setHorizontalHeaderItem(9, new QTableWidgetItem("Scholarship (BYN)"));
        
        // Update header style for new column
        studentTable->horizontalHeader()->setStyleSheet(
            "QHeaderView::section {"
            "background-color: #2A2A2A;"
            "color: #EAEAEA;"
            "padding: 12px;"
            "font-weight: 600;"
            "border: none;"
            "border-bottom: 2px solid #0d7377;"
            "font-size: 12px;"
            "}"
            "QHeaderView::section:first {"
            "background-color: transparent;"
            "color: #EAEAEA;"
            "border-bottom: none;"
            "}"
        );
        
        // Make last column stretch to fill remaining space
        studentTable->horizontalHeader()->setStretchLastSection(true);
    }
    
    int rowNum = 1;
    for (const auto& student : studentList) {
        if (!student) continue;
        
        int row = studentTable->rowCount();
        studentTable->insertRow(row);

        double avgGrade = student->getAverageGrade();
        double scholarship = student->getScholarship(); 

        // Numbering column (transparent background)
        QTableWidgetItem* numItem = new QTableWidgetItem(QString::number(rowNum++));
        numItem->setTextAlignment(Qt::AlignCenter);
        numItem->setBackground(QBrush(QColor(0, 0, 0, 0))); // Transparent
        numItem->setForeground(QBrush(QColor(180, 180, 180))); // Light gray text
        numItem->setFlags(numItem->flags() & ~Qt::ItemIsSelectable); // Not selectable
        studentTable->setItem(row, 0, numItem);
        
        QTableWidgetItem* nameItem = new QTableWidgetItem(QString::fromStdString(student->getName()));
        nameItem->setForeground(QBrush(QColor(234, 234, 234)));
        studentTable->setItem(row, 1, nameItem);
        
        QTableWidgetItem* surnameItem = new QTableWidgetItem(QString::fromStdString(student->getSurname()));
        surnameItem->setForeground(QBrush(QColor(234, 234, 234)));
        studentTable->setItem(row, 2, surnameItem);
        
        QTableWidgetItem* courseItem = new QTableWidgetItem(QString::number(student->getCourse()));
        courseItem->setForeground(QBrush(QColor(234, 234, 234)));
        courseItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        studentTable->setItem(row, 3, courseItem);
        
        QTableWidgetItem* semesterItem = new QTableWidgetItem(QString::number(student->getSemester()));
        semesterItem->setForeground(QBrush(QColor(234, 234, 234)));
        semesterItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        studentTable->setItem(row, 4, semesterItem);

        QString funding = student->getIsBudget() ? "Budget" : "Paid";
        QTableWidgetItem* fundingItem = new QTableWidgetItem(funding);
        fundingItem->setForeground(QBrush(QColor(234, 234, 234)));
        studentTable->setItem(row, 5, fundingItem);
        
        QTableWidgetItem* gradeItem = new QTableWidgetItem(QString::number(avgGrade, 'f', 2));
        gradeItem->setForeground(QBrush(QColor(234, 234, 234)));
        gradeItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        studentTable->setItem(row, 6, gradeItem);
        
        QTableWidgetItem* missedItem = new QTableWidgetItem(QString::number(student->getMissedHours()));
        missedItem->setForeground(QBrush(QColor(234, 234, 234)));
        missedItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        studentTable->setItem(row, 7, missedItem);
        
        QTableWidgetItem* socialItem = new QTableWidgetItem(student->getHasSocialScholarship() ? "Yes" : "No");
        socialItem->setForeground(QBrush(QColor(234, 234, 234)));
        socialItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        studentTable->setItem(row, 8, socialItem);
        
        // Only add scholarship column if calculated
        if (scholarshipsCalculated) {
        QTableWidgetItem* scholarshipItem = new QTableWidgetItem(QString::number(scholarship, 'f', 2));
            scholarshipItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        if (scholarship > 0) {
                scholarshipItem->setForeground(QBrush(QColor(76, 175, 80)));
                scholarshipItem->setFont(QFont("", -1, QFont::Bold));
            } else {
                scholarshipItem->setForeground(QBrush(QColor(158, 158, 158)));
            }
            studentTable->setItem(row, 9, scholarshipItem);
        }
    }
    
    // Resize columns to content except the last one (if it's the scholarship column)
    if (studentTable->columnCount() == 10 && scholarshipsCalculated) {
        // Resize all columns except last
        for (int i = 0; i < 9; ++i) {
            studentTable->resizeColumnToContents(i);
        }
        // Last column (scholarship) will stretch automatically
    } else {
    studentTable->resizeColumnsToContents();
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
    totalScholarshipLabel->setText(QString("Total Scholarships: %1 BYN").arg(totalScholarship, 0, 'f', 2));
    
    // Detailed statistics
    std::ostringstream oss;
    oss << "═══════════════════════════════════════════════════════════\n";
    oss << "           DETAILED SCHOLARSHIP STATISTICS\n";
    oss << "═══════════════════════════════════════════════════════════\n\n";
    
    // Statistics by course
    std::map<int, std::vector<std::shared_ptr<Student>>> byCourse;
    for (const auto& student : allStudents) {
        byCourse[student->getCourse()].push_back(student);
    }
    
    for (const auto& pair : byCourse) {
        oss << "Course " << pair.first << ":\n";
        oss << "  Students: " << pair.second.size() << "\n";
        double courseTotal = 0.0;
        for (const auto& s : pair.second) {
            courseTotal += s->getScholarship();
        }
        oss << "  Total Scholarships: " << std::fixed << std::setprecision(2) << courseTotal << " BYN\n\n";
    }
    
    // Students with maximum scholarship
    if (!allStudents.empty()) {
        auto maxStudent = *std::max_element(allStudents.begin(), allStudents.end(),
            [](const auto& a, const auto& b) {
                return a->getScholarship() < b->getScholarship();
            });
        
        if (maxStudent->getScholarship() > 0) {
            oss << "Maximum Scholarship:\n";
            oss << "  " << maxStudent->getSurname() << " " << maxStudent->getName();
            oss << " - " << std::fixed << std::setprecision(2) << maxStudent->getScholarship() << " BYN\n";
        }
    }
    
    statisticsText->setPlainText(QString::fromStdString(oss.str()));
}

void MainWindow::updateSemesterRange(int course) {
    int minSem = (course - 1) * 2 + 1; 
    int maxSem = minSem + 1;          
    if (semesterSpinBox) {
    semesterSpinBox->setRange(minSem, maxSem);
    if (semesterSpinBox->value() < minSem || semesterSpinBox->value() > maxSem) {
        semesterSpinBox->setValue(minSem);
    }
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
        "}"
    );
    QFormLayout* form = new QFormLayout(&dlg);
    form->setSpacing(15);
    form->setContentsMargins(20, 20, 20, 20);

    QLineEdit* nameField = new QLineEdit(QString::fromStdString(student->getName()), &dlg);
    QLineEdit* surnameField = new QLineEdit(QString::fromStdString(student->getSurname()), &dlg);
    QSpinBox* courseField = new QSpinBox(&dlg);
    courseField->setRange(1, 4);
    courseField->setValue(student->getCourse());
    QSpinBox* semesterField = new QSpinBox(&dlg);
    semesterField->setRange(1, 8);
    semesterField->setValue(student->getSemester());
    auto updateSemRange = [&](int c){ int minS=(c-1)*2+1; semesterField->setRange(minS, minS+1); if(semesterField->value()<minS||semesterField->value()>minS+1) semesterField->setValue(minS); };
    QObject::connect(courseField, QOverload<int>::of(&QSpinBox::valueChanged), &dlg, updateSemRange);
    updateSemRange(courseField->value());

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
    form->addRow("Course:", courseField);
    form->addRow("Semester:", semesterField);
    form->addRow("Average Grade:", avgField);
    form->addRow("Funding Type:", fundingField);
    form->addRow("Missed Hours:", missedHoursField);
    form->addRow("", socialCheckBox);

    QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    bb->button(QDialogButtonBox::Ok)->setText("Save");
    bb->button(QDialogButtonBox::Cancel)->setText("Cancel");
    bb->button(QDialogButtonBox::Ok)->setStyleSheet(
        "QPushButton { background-color: #4CAF50; color: white; padding: 8px 20px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #45a049; }"
    );
    bb->button(QDialogButtonBox::Cancel)->setStyleSheet(
        "QPushButton { background-color: #f44336; color: white; padding: 8px 20px; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #da190b; }"
    );
    form->addRow(bb);
    QObject::connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        if (nameField->text().trimmed().isEmpty() || surnameField->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Error", "Name and surname cannot be empty.");
            return;
        }
        
        try {
            // Save current semester data before changing
            int oldSemester = student->getSemester();
            double oldGrade = student->getAverageGrade();
            
            // If semester changed, save old semester data to history
            if (oldSemester != semesterField->value() && oldSemester > 0) {
                student->addPreviousGrade(oldSemester, oldGrade);
            }
            
        student->setName(nameField->text().trimmed().toStdString());
        student->setSurname(surnameField->text().trimmed().toStdString());
        student->setCourse(courseField->value());
        student->setSemester(semesterField->value());
        student->setAverageGrade(avgField->value());
        student->setIsBudget(fundingField->currentText() == "Budget");
            student->setMissedHours(missedHoursField->value());
            student->setHasSocialScholarship(socialCheckBox->isChecked());
            // Don't recalculate scholarship here - only when user clicks calculate button
            
            // Show warning if scholarships were already calculated
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
            
            // Show warning if scholarships were already calculated
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

void MainWindow::changeFundingType() {
    int row = studentTable->currentRow();
    if (row < 0 || row >= static_cast<int>(currentView.size())) {
        QMessageBox::warning(this, "Error", "Please select a student.");
        return;
    }
    auto student = currentView[row];
    if (!student) return;
    
    QString currentType = student->getIsBudget() ? "Budget" : "Paid";
    QString newType = student->getIsBudget() ? "Paid" : "Budget";
    
    int ret = QMessageBox::question(this, "Change Funding Type",
        QString("Change funding type from \"%1\" to \"%2\"?\n\n"
                "Note: When switching to paid education, student will lose scholarship.")
            .arg(currentType).arg(newType),
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        try {
            student->setIsBudget(newType == "Budget");
            
            // Show warning if scholarships were already calculated
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
            
            QMessageBox::information(this, "Success", "Funding type changed.");
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", e.what());
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
    historyDialog.setWindowTitle(QString("History: %1 %2").arg(
        QString::fromStdString(student->getName()),
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
        "}"
        "QTextEdit { "
        "background-color: #1e1e1e; "
        "color: #ffffff; "
        "border: 2px solid #0d7377; "
        "border-radius: 4px; "
        "padding: 10px; "
        "font-size: 12px; "
        "font-family: 'Courier New', monospace; "
        "}"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(&historyDialog);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);
    
    // Current semester info
    QLabel* currentInfoLabel = new QLabel("Current Semester Information:", &historyDialog);
    currentInfoLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #14a085; background: none; background-color: rgba(0,0,0,0); border: 0px; padding: 0px;");
    layout->addWidget(currentInfoLabel);
    
    QTextEdit* currentInfo = new QTextEdit(&historyDialog);
    currentInfo->setReadOnly(true);
    std::ostringstream currentOss;
    currentOss << "Course: " << student->getCourse() << "\n";
    currentOss << "Semester: " << student->getSemester() << "\n";
    currentOss << "Average Grade: " << std::fixed << std::setprecision(2) << student->getAverageGrade() << "\n";
    currentOss << "Funding Type: " << (student->getIsBudget() ? "Budget" : "Paid") << "\n";
    currentOss << "Missed Hours: " << student->getMissedHours() << "\n";
    currentOss << "Social Scholarship: " << (student->getHasSocialScholarship() ? "Yes" : "No") << "\n";
    currentOss << "Scholarship: " << std::fixed << std::setprecision(2) << student->getScholarship() << " BYN\n";
    currentInfo->setPlainText(QString::fromStdString(currentOss.str()));
    currentInfo->setMaximumHeight(150);
    layout->addWidget(currentInfo);
    
    // Previous semesters history
    QLabel* historyLabel = new QLabel("Previous Semesters History:", &historyDialog);
    historyLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #14a085; margin-top: 10px; background: none; background-color: rgba(0,0,0,0); border: 0px; padding: 0px;");
    layout->addWidget(historyLabel);
    
    QTextEdit* historyText = new QTextEdit(&historyDialog);
    historyText->setReadOnly(true);
    
    const auto& previousGrades = student->getPreviousSemesterGrades();
    if (previousGrades.empty()) {
        historyText->setPlainText("No previous semester data available.\nThis student is on their first semester.");
    } else {
        std::ostringstream historyOss;
        historyOss << "════════════════════════════════════════════════\n";
        historyOss << "Semester History\n";
        historyOss << "════════════════════════════════════════════════\n\n";
        
        // Sort by semester number
        std::vector<std::pair<int, double>> sortedHistory;
        for (const auto& pair : previousGrades) {
            sortedHistory.push_back(pair);
        }
        std::sort(sortedHistory.begin(), sortedHistory.end());
        
        for (const auto& pair : sortedHistory) {
            int sem = pair.first;
            double grade = pair.second;
            int course = (sem - 1) / 2 + 1;
            
            historyOss << "Course " << course << ", Semester " << sem << ":\n";
            historyOss << "  Average Grade: " << std::fixed << std::setprecision(2) << grade << "\n";
            
            // Calculate what scholarship would have been
            double prevScholarship = 0.0;
            if (student->getIsBudget()) {
                prevScholarship = ScholarshipCalculator::calculateScholarship(grade);
            }
            historyOss << "  Scholarship (est.): " << std::fixed << std::setprecision(2) << prevScholarship << " BYN\n";
            historyOss << "\n";
        }
        
        historyText->setPlainText(QString::fromStdString(historyOss.str()));
    }
    
    layout->addWidget(historyText, 1);
    
    // Close button
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
        "QPushButton:hover { background-color: #14a085; }"
    );
    connect(closeButton, &QPushButton::clicked, &historyDialog, &QDialog::accept);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);
    
    historyDialog.exec();
}
