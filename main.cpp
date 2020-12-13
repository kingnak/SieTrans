#include "sietranswnd.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("SieTrans");
    a.setOrganizationName("SieTrans");
    SieTransWnd w;
    w.show();
    return a.exec();
}
