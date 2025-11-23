#include "studentTableManager.h"

#include <QAbstractItemView>
#include <QBrush>
#include <QFont>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVariant>
#include <QWidget>

#include "student.h"

StudentTableManager::StudentTableManager(QTableWidget* tableWidget, QObject* parent)
    : QObject(parent), table(tableWidget) {}

void StudentTableManager::configure(QObject* eventFilterOwner) {
    if (!table) return;

    table->setColumnCount(10);
    QStringList headers = {"#",        "Name",         "Surname",       "Course",
                           "Semester", "Funding Type", "Average Grade", "Missed Hours",
                           "Social", "Actions"};
    table->setHorizontalHeaderLabels(headers);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setStretchLastSection(false);
    table->setSortingEnabled(false);
    if (table->horizontalHeader() && eventFilterOwner) {
        table->horizontalHeader()->installEventFilter(eventFilterOwner);
    }

    // Улучшенные шрифты
    QFont baseFont("SF Pro Display", 11, QFont::Normal); // Более современный и четкий
    baseFont.setStyleHint(QFont::SansSerif);
    table->setFont(baseFont);
    
    if (table->horizontalHeader()) {
        QFont headerFont("", 11, QFont::DemiBold); // Полужирный для заголовков
        headerFont.setStyleHint(QFont::SansSerif);
        table->horizontalHeader()->setFont(headerFont);
    }

    table->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "background-color: #2A2A2A;"
        "color: #EAEAEA;"
        "padding: 14px 12px;" // Уменьшил отступы
        "border: none;"
        "border-bottom: 2px solid #0d7377;"
        "min-height: 45px;" // Уменьшил высоту
        "}"
        "QHeaderView::section:first {"
        "background-color: transparent;"
        "color: #EAEAEA;"
        "border-bottom: none;"
        "}");

    table->setStyleSheet(
        "QTableWidget {"
        "gridline-color: #2d2d2d;"
        "background-color: #1B1B1B;"
        "alternate-background-color: #1E1E1E;"
        "border: 2px solid #0d7377;"
        "border-radius: 8px;"
        "}"
        "QTableWidget::item {"
        "padding: 12px 10px;" // Уменьшил отступы в ячейках
        "border: none;"
        "}"
        "QTableWidget::item:selected {"
        "background-color: #0d7377 !important;"
        "color: white !important;"
        "}"
        "QTableWidget::item:selected:alternate {"
        "background-color: #0d7377 !important;"
        "color: white !important;"
        "}"
        "QTableWidget::item:alternate {"
        "background-color: #1E1E1E;"
        "}");

    connect(table, &QTableWidget::itemSelectionChanged, this,
            &StudentTableManager::updateSelectionVisual);
    connect(table, &QTableWidget::currentItemChanged, this,
            [this](QTableWidgetItem*, QTableWidgetItem*) { updateSelectionVisual(); });
}

void StudentTableManager::ensureScholarshipColumn(bool scholarshipsCalculated) {
    if (!table) return;
    int baseColumns = 10; // 9 data columns + 1 Actions column
    if (scholarshipsCalculated && table->columnCount() == baseColumns) {
        table->insertColumn(baseColumns);
        table->setHorizontalHeaderItem(baseColumns, new QTableWidgetItem("Scholarship (BYN)"));
        table->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
        table->horizontalHeader()->setStretchLastSection(true);
    } else if (!scholarshipsCalculated && table->columnCount() == baseColumns + 1) {
        table->removeColumn(baseColumns);
        table->horizontalHeader()->setStretchLastSection(false);
    }
}

void StudentTableManager::populate(const std::vector<std::shared_ptr<Student>>& students,
                                   bool scholarshipsCalculated) {
    if (!table) return;

    ensureScholarshipColumn(scholarshipsCalculated);

    table->blockSignals(true);
    table->setRowCount(0);

    int rowNum = 1;
    for (const auto& student : students) {
        if (!student) continue;

        int row = table->rowCount();
        table->insertRow(row);

        double avgGrade = student->getAverageGrade();
        double scholarship = student->getScholarship();

        QColor defaultTextColor(234, 234, 234);
        QFont itemFont;
        itemFont.setPointSize(11); // Уменьшил размер
        itemFont.setFamily("SF Pro Display");
        itemFont.setStyleHint(QFont::SansSerif);

        // Номер строки
        QTableWidgetItem* numItem = new QTableWidgetItem(QString::number(rowNum++));
        numItem->setTextAlignment(Qt::AlignCenter);
        numItem->setBackground(QBrush(QColor(0, 0, 0, 0)));
        numItem->setForeground(QBrush(QColor(180, 180, 180)));
        numItem->setFlags(numItem->flags() & ~Qt::ItemIsSelectable);
        numItem->setData(Qt::UserRole, QVariant());
        numItem->setFont(itemFont);
        table->setItem(row, 0, numItem);

        // Имя и Фамилия - делаем шрифт немного жирнее для лучшей читаемости
        QFont nameFont = itemFont;
        nameFont.setWeight(QFont::Medium);

        QTableWidgetItem* nameItem = new QTableWidgetItem(QString::fromStdString(student->getName()));
        nameItem->setForeground(QBrush(defaultTextColor));
        nameItem->setFont(nameFont);
        table->setItem(row, 1, nameItem);

        QTableWidgetItem* surnameItem = new QTableWidgetItem(QString::fromStdString(student->getSurname()));
        surnameItem->setForeground(QBrush(defaultTextColor));
        surnameItem->setFont(nameFont);
        table->setItem(row, 2, surnameItem);

        // Остальные ячейки
        QTableWidgetItem* courseItem = new QTableWidgetItem(QString::number(student->getCourse()));
        courseItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        courseItem->setForeground(QBrush(defaultTextColor));
        courseItem->setFont(itemFont);
        table->setItem(row, 3, courseItem);

        QTableWidgetItem* semesterItem = new QTableWidgetItem(QString::number(student->getSemester()));
        semesterItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        semesterItem->setForeground(QBrush(defaultTextColor));
        semesterItem->setFont(itemFont);
        table->setItem(row, 4, semesterItem);

        QString funding = student->getIsBudget() ? "Budget" : "Paid";
        QTableWidgetItem* fundingItem = new QTableWidgetItem(funding);
        fundingItem->setForeground(QBrush(defaultTextColor));
        fundingItem->setFont(itemFont);
        table->setItem(row, 5, fundingItem);

        QTableWidgetItem* gradeItem = new QTableWidgetItem(QString::number(avgGrade, 'f', 2));
        gradeItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        gradeItem->setForeground(QBrush(defaultTextColor));
        gradeItem->setFont(itemFont);
        table->setItem(row, 6, gradeItem);

        QTableWidgetItem* missedItem = new QTableWidgetItem(QString::number(student->getMissedHours()));
        missedItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        QColor redColor(255, 100, 100); // Более мягкий красный
        if (student->getMissedHours() >= 12) {
            missedItem->setData(Qt::UserRole + 10, true);
            missedItem->setData(Qt::UserRole + 11, redColor);
            missedItem->setForeground(QBrush(redColor));
            QFont boldFont = itemFont;
            boldFont.setWeight(QFont::Bold);
            missedItem->setFont(boldFont);
        } else {
            missedItem->setData(Qt::UserRole + 10, false);
            missedItem->setData(Qt::UserRole + 11, QColor());
            missedItem->setForeground(QBrush(defaultTextColor));
            missedItem->setFont(itemFont);
        }
        table->setItem(row, 7, missedItem);

        QTableWidgetItem* socialItem = new QTableWidgetItem(student->getHasSocialScholarship() ? "Yes" : "No");
        socialItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        socialItem->setData(Qt::UserRole, 0);
        socialItem->setForeground(QBrush(defaultTextColor));
        socialItem->setFont(itemFont);
        table->setItem(row, 8, socialItem);

        // Actions column with icon buttons
        QWidget* actionsWidget = createActionButtons(row);
        table->setCellWidget(row, 9, actionsWidget);

        if (scholarshipsCalculated && table->columnCount() > 10) {
            QTableWidgetItem* scholarshipItem = new QTableWidgetItem(QString::number(scholarship, 'f', 2));
            scholarshipItem->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            scholarshipItem->setData(Qt::UserRole, 0);
            scholarshipItem->setData(Qt::UserRole + 2, scholarship);
            
            QFont scholarshipFont = itemFont;
            if (scholarship > 0) {
                scholarshipItem->setData(Qt::UserRole + 1, QColor(100, 230, 100)); // Яркий зеленый
                scholarshipFont.setWeight(QFont::DemiBold);
                scholarshipItem->setFont(scholarshipFont);
                scholarshipItem->setForeground(QBrush(QColor(100, 230, 100)));
            } else {
                scholarshipItem->setData(Qt::UserRole + 1, QColor(180, 180, 180));
                scholarshipItem->setFont(scholarshipFont);
                scholarshipItem->setForeground(QBrush(QColor(180, 180, 180)));
            }
            table->setItem(row, 10, scholarshipItem);
        }
    }

    // Настройка размеров колонок
    if (table->columnCount() == 11 && scholarshipsCalculated) {
        for (int i = 0; i < 10; ++i) {
            table->resizeColumnToContents(i);
        }
        table->horizontalHeader()->setStretchLastSection(true);
    } else {
        table->resizeColumnsToContents();
    }

    // Установим минимальные ширины для лучшего отображения
    table->setColumnWidth(1, 100); // Name
    table->setColumnWidth(2, 120); // Surname
    table->setColumnWidth(5, 100); // Funding Type
    table->setColumnWidth(6, 110); // Average Grade
    table->setColumnWidth(7, 100); // Missed Hours
    table->setColumnWidth(8, 70);  // Social
    table->setColumnWidth(9, 120); // Actions

    updateRowNumbers();
    table->blockSignals(false);
    updateSelectionVisual();
    applyMissedHoursStyling();
}

// Остальные методы остаются без изменений...
void StudentTableManager::applyMissedHoursStyling() {
    if (!table) return;
    for (int row = 0; row < table->rowCount(); ++row) {
        QTableWidgetItem* missedItem = table->item(row, 7);
        if (missedItem && missedItem->data(Qt::UserRole + 10).toBool()) {
            QColor redColor = missedItem->data(Qt::UserRole + 11).value<QColor>();
            if (!redColor.isValid()) redColor = QColor(255, 100, 100);
            missedItem->setForeground(QBrush(redColor));
            QFont font = missedItem->font();
            font.setWeight(QFont::Bold);
            missedItem->setFont(font);
        } else if (missedItem) {
            QColor defaultTextColor(234, 234, 234);
            missedItem->setForeground(QBrush(defaultTextColor));
            QFont font = missedItem->font();
            font.setWeight(QFont::Normal);
            missedItem->setFont(font);
        }
    }
}

void StudentTableManager::updateSelectionVisual() {
    if (!table) return;

    QColor selectedColor(13, 115, 119);
    QColor defaultColor(234, 234, 234);

    for (int row = 0; row < table->rowCount(); ++row) {
        bool isSelected = table->selectionModel() && table->selectionModel()->isRowSelected(row);

        for (int col = 0; col < table->columnCount(); ++col) {
            // Skip Actions column (col 9) as it contains widgets, not items
            if (col == 9) continue;
            
            QTableWidgetItem* item = table->item(row, col);
            if (!item) continue;

            if (isSelected) {
                item->setBackground(QBrush(selectedColor));
                if (col == 7 && item->data(Qt::UserRole + 10).toBool()) {
                    QColor redColor = item->data(Qt::UserRole + 11).value<QColor>();
                    if (!redColor.isValid()) redColor = QColor(255, 100, 100);
                    item->setForeground(QBrush(redColor));
                } else {
                    item->setForeground(QBrush(Qt::white));
                }
            } else {
                item->setBackground(QBrush(Qt::transparent));
                if (col == 0) {
                    item->setForeground(QBrush(QColor(180, 180, 180)));
                } else if (col == 7 && item->data(Qt::UserRole + 10).toBool()) {
                    QColor redColor = item->data(Qt::UserRole + 11).value<QColor>();
                    if (!redColor.isValid()) redColor = QColor(255, 100, 100);
                    item->setForeground(QBrush(redColor));
                } else if (col == 7) {
                    item->setForeground(QBrush(defaultColor));
                } else if (col == 10 && item->data(Qt::UserRole + 2).isValid()) {
                    // Scholarship column
                    QColor storedColor = item->data(Qt::UserRole + 1).value<QColor>();
                    item->setForeground(QBrush(storedColor.isValid() ? storedColor : defaultColor));
                } else if (col == 9) {
                    // Actions column - skip, it's a widget
                    continue;
                } else {
                    item->setForeground(QBrush(defaultColor));
                }
            }
        }
    }
}

void StudentTableManager::updateRowNumbers() {
    if (!table) return;
    for (int row = 0; row < table->rowCount(); ++row) {
        QTableWidgetItem* numItem = table->item(row, 0);
        if (numItem) {
            numItem->setText(QString::number(row + 1));
        } else {
            numItem = new QTableWidgetItem(QString::number(row + 1));
            numItem->setTextAlignment(Qt::AlignCenter);
            numItem->setBackground(QBrush(QColor(0, 0, 0, 0)));
            numItem->setForeground(QBrush(QColor(180, 180, 180)));
            numItem->setFlags(numItem->flags() & ~Qt::ItemIsSelectable);
            
            QFont font;
            font.setPointSize(10);
            font.setFamily("Segoe UI");
            numItem->setFont(font);
            
            table->setItem(row, 0, numItem);
        }
    }
}

QWidget* StudentTableManager::createActionButtons(int row) {
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(5, 2, 5, 2);
    layout->setSpacing(5);
    layout->setAlignment(Qt::AlignCenter);

    // Edit button (pencil icon)
    QPushButton* editBtn = new QPushButton("✏️");
    editBtn->setToolTip("Edit Student");
    editBtn->setFixedSize(30, 30);
    editBtn->setStyleSheet(
        "QPushButton {"
        "background-color: #0d7377;"
        "color: white;"
        "border: none;"
        "border-radius: 4px;"
        "font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "background-color: #14a085;"
        "}"
        "QPushButton:pressed {"
        "background-color: #0a5d61;"
        "}");
    editBtn->setProperty("row", row);
    connect(editBtn, &QPushButton::clicked, this, &StudentTableManager::onEditClicked);
    layout->addWidget(editBtn);

    // View history button (eye icon)
    QPushButton* viewBtn = new QPushButton("👁️");
    viewBtn->setToolTip("View History");
    viewBtn->setFixedSize(30, 30);
    viewBtn->setStyleSheet(
        "QPushButton {"
        "background-color: #0d7377;"
        "color: white;"
        "border: none;"
        "border-radius: 4px;"
        "font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "background-color: #14a085;"
        "}"
        "QPushButton:pressed {"
        "background-color: #0a5d61;"
        "}");
    viewBtn->setProperty("row", row);
    connect(viewBtn, &QPushButton::clicked, this, &StudentTableManager::onViewClicked);
    layout->addWidget(viewBtn);

    // Delete button (cross/X icon)
    QPushButton* deleteBtn = new QPushButton("❌");
    deleteBtn->setToolTip("Delete Student");
    deleteBtn->setFixedSize(30, 30);
    deleteBtn->setStyleSheet(
        "QPushButton {"
        "background-color: #d32f2f;"
        "color: white;"
        "border: none;"
        "border-radius: 4px;"
        "font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "background-color: #f44336;"
        "}"
        "QPushButton:pressed {"
        "background-color: #b71c1c;"
        "}");
    deleteBtn->setProperty("row", row);
    connect(deleteBtn, &QPushButton::clicked, this, &StudentTableManager::onDeleteClicked);
    layout->addWidget(deleteBtn);

    return widget;
}

void StudentTableManager::onEditClicked() {
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (btn) {
        int row = btn->property("row").toInt();
        emit editStudentRequested(row);
    }
}

void StudentTableManager::onDeleteClicked() {
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (btn) {
        int row = btn->property("row").toInt();
        emit deleteStudentRequested(row);
    }
}

void StudentTableManager::onViewClicked() {
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (btn) {
        int row = btn->property("row").toInt();
        emit viewHistoryRequested(row);
    }
}