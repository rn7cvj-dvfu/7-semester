#include "mainwindow/mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("Temperature Monitor");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Lab6");

    MainWindow window;
    window.show();

    return app.exec();
}
