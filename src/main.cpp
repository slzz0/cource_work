#include <QApplication>
#include <QStyleFactory>

#include "mainWindow.h"


int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("Scholarship Management System");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("University (BSUIR)");

    app.setStyle(QStyleFactory::create("Fusion"));

    MainWindow window;
    window.show();

    return app.exec();
}