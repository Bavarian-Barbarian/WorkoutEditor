#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.setWindowIcon(QIcon(":/images/icons/workouteditor.png"));
    w.showMaximized();

    return a.exec();
}
