#include <QApplication>
#include <QCoreApplication>
#include <QStyleFactory>

#include "ui/mainWindow.h"


int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName("Scholarship Management System");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setOrganizationName("University (BSUIR)");

    QApplication::setStyle(QStyleFactory::create("Fusion"));

    MainWindow window;
    window.show();

    return QApplication::exec();
}