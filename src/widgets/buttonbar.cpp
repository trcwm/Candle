#include "buttonbar.h"

namespace GUI
{


ButtonBar::ButtonBar(QWidget *parent) : QToolBar(parent)
{
    createWidget();
}

void ButtonBar::createWidget()
{
    addAction(QIcon(":/images/restart.png"), "Restart");
    addAction(QIcon(":/images/unlock.png"), "Unlock");
    addSeparator();
    addAction(QIcon(":/images/search_for_home2.png"), "Home");
    addAction(QIcon(":/images/search_for_z.png"), "Probe Z");
    addSeparator();
    addAction(QIcon(":/images/axis_zero.png"), "Zero XY");
    addAction(QIcon(":/images/zero_z.png"), "Zero Z");
    addSeparator();
    addAction(QIcon(":/images/origin.png"), "Restore Origin");
    addAction(QIcon(":/images/run.png"), "Safe Position");
    addSeparator();
    addAction(QIcon(":/images/num1.png"), "User 1");
    addAction(QIcon(":/images/num2.png"), "User 2");
    addAction(QIcon(":/images/num3.png"), "User 3");
    addAction(QIcon(":/images/num4.png"), "User 4");
}

};
