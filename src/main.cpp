#include <QApplication>
#include <QStyle>
#include <QStyleFactory>
#include "gui/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // Configure Application property
    QApplication::setStyle(QStyleFactory::create("fusion"));
    QApplication::setApplicationName("Finback");
    QApplication::setApplicationDisplayName("Finback");
    QApplication::setApplicationVersion("8.0 Prototype");
    // Configure main window.
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
