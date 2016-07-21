
#include <QApplication>
#include <QMessageBox>
#include "dcrwidget.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    DCResistance  *widget = new DCResistance;
    widget->show();
    
    return app.exec();
}
