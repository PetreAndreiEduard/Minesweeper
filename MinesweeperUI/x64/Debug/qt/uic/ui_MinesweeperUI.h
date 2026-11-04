/********************************************************************************
** Form generated from reading UI file 'MinesweeperUI.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MINESWEEPERUI_H
#define UI_MINESWEEPERUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MinesweeperUIClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MinesweeperUIClass)
    {
        if (MinesweeperUIClass->objectName().isEmpty())
            MinesweeperUIClass->setObjectName("MinesweeperUIClass");
        MinesweeperUIClass->resize(600, 400);
        menuBar = new QMenuBar(MinesweeperUIClass);
        menuBar->setObjectName("menuBar");
        MinesweeperUIClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MinesweeperUIClass);
        mainToolBar->setObjectName("mainToolBar");
        MinesweeperUIClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(MinesweeperUIClass);
        centralWidget->setObjectName("centralWidget");
        MinesweeperUIClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(MinesweeperUIClass);
        statusBar->setObjectName("statusBar");
        MinesweeperUIClass->setStatusBar(statusBar);

        retranslateUi(MinesweeperUIClass);

        QMetaObject::connectSlotsByName(MinesweeperUIClass);
    } // setupUi

    void retranslateUi(QMainWindow *MinesweeperUIClass)
    {
        MinesweeperUIClass->setWindowTitle(QCoreApplication::translate("MinesweeperUIClass", "MinesweeperUI", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MinesweeperUIClass: public Ui_MinesweeperUIClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MINESWEEPERUI_H
