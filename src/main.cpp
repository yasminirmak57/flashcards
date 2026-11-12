#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setOrganizationName("Flashcards");
    QApplication::setApplicationName("Flashcards");
    MainWindow w;
    w.show();
    return app.exec();
}
