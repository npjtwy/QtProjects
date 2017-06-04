#include "toolbox1.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ToolBox1 box(NULL);
    box.resize(300, 600);
    box.setWindowTitle(box.getThisUserName());

    box.show();

    return a.exec();
}
