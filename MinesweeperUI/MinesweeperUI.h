#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MinesweeperUI.h"

class MinesweeperUI : public QMainWindow
{
    Q_OBJECT

public:
    MinesweeperUI(QWidget *parent = nullptr);
    ~MinesweeperUI();

private:
    Ui::MinesweeperUIClass ui;
};

