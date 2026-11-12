#include "mainwindow.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QApplication::setOrganizationName("Flashcards");
    QApplication::setApplicationName("Flashcards");
    QApplication::setWindowIcon(QIcon(":/icon/icon.png"));
    MainWindow w;
    w.show();
    return app.exec();
}
