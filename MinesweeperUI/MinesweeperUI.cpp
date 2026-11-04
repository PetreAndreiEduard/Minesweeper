#include "MinesweeperUI.h"
#include "IMinesweeperGame.h"

MinesweeperUI::MinesweeperUI(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
}

MinesweeperUI::~MinesweeperUI()
{}

