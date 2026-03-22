/********************************************************************************
** Form generated from reading UI file 'cell.ui'
**
** Created by: Qt User Interface Compiler version 6.10.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CELL_H
#define UI_CELL_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_cell
{
public:
    QWidget *centralwidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *cell)
    {
        if (cell->objectName().isEmpty())
            cell->setObjectName("cell");
        cell->resize(800, 600);
        centralwidget = new QWidget(cell);
        centralwidget->setObjectName("centralwidget");
        cell->setCentralWidget(centralwidget);
        menubar = new QMenuBar(cell);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 18));
        cell->setMenuBar(menubar);
        statusbar = new QStatusBar(cell);
        statusbar->setObjectName("statusbar");
        cell->setStatusBar(statusbar);

        retranslateUi(cell);

        QMetaObject::connectSlotsByName(cell);
    } // setupUi

    void retranslateUi(QMainWindow *cell)
    {
        cell->setWindowTitle(QCoreApplication::translate("cell", "cell", nullptr));
    } // retranslateUi

};

namespace Ui {
    class cell: public Ui_cell {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CELL_H
