#include "mainwindow.h"
#include <QTranslator>
#include <QApplication>
#include <QLocale>
#include <QDebug>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);


    MainWindow w;
    w.show();

    return app.exec();
}


