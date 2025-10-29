#include "mainWindow.h"
#include <QApplication>
#include <QStringList>
#include <QBrush>
#include <sstream>
#include <iomanip>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupUI();
    showAllStudents();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    setWindowTitle("Scholarship Management System");
    setMinimumSize(1200, 700);
    resize(1200, 700);

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    statisticsLabel = new QLabel("Total Students: 0", this);
    statisticsLabel->setStyleSheet("font-size: 14px; font-weight: bold; padding: 5px;");
    mainLayout->addWidget(statisticsLabel);

    createTabs();
    mainLayout->addWidget(tabWidget);

    createStudentTable();
    mainLayout->addWidget(studentTable, 1);

    createActionButtons();
    mainLayout->addLayout(new QHBoxLayout());

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(showAllButton);
    buttonLayout->addWidget(calculateScholarshipsButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    updateStatistics();
}

void MainWindow::createTabs() {
    tabWidget = new QTabWidget(this);
    QWidget* addTab = new QWidget(this);
    QWidget* searchTab = new QWidget(this);
    
    createAddStudentTab(addTab);
    createSearchTab(searchTab);
    
    tabWidget->addTab(addTab, "Add Student");
    tabWidget->addTab(searchTab, "Search");
}

void MainWindow::createAddStudentTab(QWidget* tab) {
    QGridLayout* layout = new QGridLayout(tab);

    layout->addWidget(new QLabel("Name:"), 0, 0);
    nameEdit = new QLineEdit(this);
    nameEdit->setPlaceholderText("Enter student name");
    layout->addWidget(nameEdit, 0, 1);

    layout->addWidget(new QLabel("Surname:"), 1, 0);
    surnameEdit = new QLineEdit(this);
    surnameEdit->setPlaceholderText("Enter student surname");
    layout->addWidget(surnameEdit, 1, 1);

    layout->addWidget(new QLabel("Course:"), 2, 0);
    courseSpinBox = new QSpinBox(this);
    courseSpinBox->setMinimum(1);
    courseSpinBox->setMaximum(4);
    courseSpinBox->setValue(1);
    layout->addWidget(courseSpinBox, 2, 1);

    layout->addWidget(new QLabel("Semester:"), 3, 0);
    semesterSpinBox = new QSpinBox(this);
    semesterSpinBox->setMinimum(1);
    semesterSpinBox->setMaximum(8);
    semesterSpinBox->setValue(1);
    layout->addWidget(semesterSpinBox, 3, 1);

    connect(courseSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onCourseChanged);
    onCourseChanged(courseSpinBox->value());

    layout->addWidget(new QLabel("Average Grade:"), 4, 0);
    averageGradeSpinBox = new QDoubleSpinBox(this);
    averageGradeSpinBox->setMinimum(0.0);
    averageGradeSpinBox->setMaximum(10.0);
    averageGradeSpinBox->setDecimals(2);
    averageGradeSpinBox->setSingleStep(0.1);
    averageGradeSpinBox->setValue(5.0);
    layout->addWidget(averageGradeSpinBox, 4, 1);

    layout->addWidget(new QLabel("Funding:"), 5, 0);
    fundingCombo = new QComboBox(this);
    fundingCombo->addItem("Budget");
    fundingCombo->addItem("Paid");
    layout->addWidget(fundingCombo, 5, 1);

    addButton = new QPushButton("Add Student", this);
    addButton->setStyleSheet(
        "QPushButton {"
        "background-color: #4CAF50;"
        "color: white;"
        "padding: 8px;"
        "border-radius: 4px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #45a049;"
        "}"
    );
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addStudent);
    layout->addWidget(addButton, 6, 0, 1, 2);
}

void MainWindow::createSearchTab(QWidget* tab) {
    QGridLayout* layout = new QGridLayout(tab);

    layout->addWidget(new QLabel("Search by Name:"), 0, 0);
    searchNameEdit = new QLineEdit(this);
    searchNameEdit->setPlaceholderText("Enter name or surname");
    layout->addWidget(searchNameEdit, 0, 1);

    searchByNameButton = new QPushButton("Search", this);
    searchByNameButton->setStyleSheet(
        "QPushButton {"
        "background-color: #2196F3;"
        "color: white;"
        "padding: 8px;"
        "border-radius: 4px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #0b7dda;"
        "}"
    );
    connect(searchByNameButton, &QPushButton::clicked, this, &MainWindow::searchByName);
    layout->addWidget(searchByNameButton, 0, 2);

    layout->addWidget(new QLabel("Search by Average Grade (exact):"), 1, 0);
    exactGradeSpinBox = new QDoubleSpinBox(this);
    exactGradeSpinBox->setMinimum(0.0);
    exactGradeSpinBox->setMaximum(10.0);
    exactGradeSpinBox->setDecimals(2);
    exactGradeSpinBox->setSingleStep(0.1);
    exactGradeSpinBox->setValue(8.00);
    layout->addWidget(exactGradeSpinBox, 1, 1);

    searchByExactGradeButton = new QPushButton("Search", this);
    searchByExactGradeButton->setStyleSheet(
        "QPushButton {"
        "background-color: #2196F3;"
        "color: white;"
        "padding: 8px;"
        "border-radius: 4px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #0b7dda;"
        "}"
    );
    connect(searchByExactGradeButton, &QPushButton::clicked, this, &MainWindow::searchByAverageGradeEqual);
    layout->addWidget(searchByExactGradeButton, 1, 2);
}

void MainWindow::createStudentTable() {
    studentTable = new QTableWidget(this);
    studentTable->setColumnCount(7);
    QStringList headers = {"Name", "Surname", "Course", "Semester", "Funding",
                          "Average Grade", "Scholarship (BYN)"};
    studentTable->setHorizontalHeaderLabels(headers);
    studentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    studentTable->setSelectionMode(QAbstractItemView::SingleSelection);
    studentTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    studentTable->setAlternatingRowColors(true);
    studentTable->horizontalHeader()->setStretchLastSection(true);
    studentTable->setStyleSheet(
        "QTableWidget {"
        "gridline-color: #d0d0d0;"
        "background-color: white;"
        "alternate-background-color: #f5f5f5;"
        "}"
        "QHeaderView::section {"
        "background-color: #2196F3;"
        "color: white;"
        "padding: 8px;"
        "font-weight: bold;"
        "border: none;"
        "}"
    );
}

void MainWindow::createActionButtons() {
    showAllButton = new QPushButton("Show All Students", this);
    showAllButton->setStyleSheet(
        "QPushButton {"
        "background-color: #FF9800;"
        "color: white;"
        "padding: 8px 16px;"
        "border-radius: 4px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #f57c00;"
        "}"
    );
    connect(showAllButton, &QPushButton::clicked, this, &MainWindow::showAllStudents);

    calculateScholarshipsButton = new QPushButton("Calculate All Scholarships", this);
    calculateScholarshipsButton->setStyleSheet(
        "QPushButton {"
        "background-color: #9C27B0;"
        "color: white;"
        "padding: 8px 16px;"
        "border-radius: 4px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #7b1fa2;"
        "}"
    );
    connect(calculateScholarshipsButton, &QPushButton::clicked, this, &MainWindow::calculateAllScholarships);

    clearButton = new QPushButton("Clear Database", this);
    clearButton->setStyleSheet(
        "QPushButton {"
        "background-color: #f44336;"
        "color: white;"
        "padding: 8px 16px;"
        "border-radius: 4px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #da190b;"
        "}"
    );
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearDatabase);
}

void MainWindow::addStudent() {
    QString name = nameEdit->text().trimmed();
    QString surname = surnameEdit->text().trimmed();
    
    if (name.isEmpty() || surname.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter both name and surname.");
        return;
    }

    double avg = averageGradeSpinBox->value();
    bool isBudget = (fundingCombo->currentText() == "Budget");
    database.addStudent(name.toStdString(), surname.toStdString(),
                       courseSpinBox->value(), semesterSpinBox->value(), avg, isBudget);

    nameEdit->clear();
    surnameEdit->clear();
    courseSpinBox->setValue(1);
    semesterSpinBox->setValue(1);
    averageGradeSpinBox->setValue(5.0);
    fundingCombo->setCurrentIndex(0);

    showAllStudents();
    updateStatistics();

    QMessageBox::information(this, "Success", "Student added successfully!");
}

void MainWindow::searchByName() {
    QString searchText = searchNameEdit->text().trimmed();
    
    if (searchText.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a name to search.");
        return;
    }

    std::vector<std::shared_ptr<Student>> results = 
        database.searchByName(searchText.toStdString());
    
    updateStudentTable(results);
    
    if (results.empty()) {
        QMessageBox::information(this, "Search Results", 
            "No students found with the given name.");
    }
}

void MainWindow::searchByAverageGradeEqual() {
    double exact = exactGradeSpinBox->value();
    std::vector<std::shared_ptr<Student>> results = database.searchByAverageEqual(exact);
    updateStudentTable(results);
    if (results.empty()) {
        QMessageBox::information(this, "Search Results",
            "No students found with the specified average grade.");
    }
}

void MainWindow::showAllStudents() {
    std::vector<std::shared_ptr<Student>> allStudents = database.getAllStudents();
    updateStudentTable(allStudents);
}

void MainWindow::calculateAllScholarships() {
    showAllStudents();
    QMessageBox::information(this, "Scholarships Calculated", 
        "All scholarships have been calculated and displayed in the table.");
}

void MainWindow::clearDatabase() {
    int ret = QMessageBox::question(this, "Confirm", 
        "Are you sure you want to clear all student data?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        database.clear();
        showAllStudents();
        updateStatistics();
        QMessageBox::information(this, "Success", "Database cleared successfully.");
    }
}

void MainWindow::updateStudentTable(const std::vector<std::shared_ptr<Student>>& studentList) {
    studentTable->setRowCount(0);
    
    for (const auto& student : studentList) {
        if (student == nullptr) continue;
        
        int row = studentTable->rowCount();
        studentTable->insertRow(row);

        double avgGrade = student->calculateAverageGrade();
        double scholarship = ScholarshipCalculator::calculateScholarshipForStudent(student.get());
        if (!student->getIsBudget()) {
            scholarship = 0.0;
        }

        studentTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(student->getName())));
        studentTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(student->getSurname())));
        studentTable->setItem(row, 2, new QTableWidgetItem(QString::number(student->getCourse())));
        studentTable->setItem(row, 3, new QTableWidgetItem(QString::number(student->getSemester())));

        QString funding = student->getIsBudget() ? "Budget" : "Paid";
        studentTable->setItem(row, 4, new QTableWidgetItem(funding));

        QTableWidgetItem* gradeItem = new QTableWidgetItem(QString::number(avgGrade, 'f', 2));
        studentTable->setItem(row, 5, gradeItem);

        QTableWidgetItem* scholarshipItem = new QTableWidgetItem(QString::number(scholarship, 'f', 2));
        if (scholarship > 0) {
            scholarshipItem->setForeground(QBrush(QColor(0, 128, 0))); // Green color
        }
        studentTable->setItem(row, 6, scholarshipItem);
    }
    
    studentTable->resizeColumnsToContents();
}

void MainWindow::updateStatistics() {
    size_t count = database.getStudentCount();
    statisticsLabel->setText(QString("Total Students: %1").arg(count));
}

void MainWindow::updateSemesterRange(int course) {
    int minSem = (course - 1) * 2 + 1; 
    int maxSem = minSem + 1;          
    semesterSpinBox->setRange(minSem, maxSem);
    if (semesterSpinBox->value() < minSem || semesterSpinBox->value() > maxSem) {
        semesterSpinBox->setValue(minSem);
    }
}

void MainWindow::onCourseChanged(int course) {
    updateSemesterRange(course);
}


