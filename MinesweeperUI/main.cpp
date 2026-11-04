#include "MinesweeperUI.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MinesweeperUI window;
    window.show();
    return app.exec();
}
