#include "mainwindow.h"

#include <QApplication>
#include <QString>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    // Optionally open a project given on the command line. A path starting with
    // ':' is a Qt resource (e.g. a built-in example); anything else is a file.
    if(argc > 1) {
        QString path = QString::fromLocal8Bit(argv[1]);
        if(path.startsWith(':')) {
            w.openFromResource(path);
        } else {
            w.openFromFile(path);
        }
    }
    return a.exec();
}
