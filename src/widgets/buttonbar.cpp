#include "buttonbar.h"
#include <QAction>

namespace GUI
{

ButtonBar::ButtonBar(QWidget *parent) : QToolBar(parent)
{
    createWidget();
}

void ButtonBar::createWidget()
{
    auto actRestart = addAction(QIcon(":/images/restart.png"), "Restart");
    auto actUnlock  = addAction(QIcon(":/images/unlock.png"), "Unlock");
    addSeparator();
    auto actHome    = addAction(QIcon(":/images/search_for_home2.png"), "Home");
    auto actProbeZ  = addAction(QIcon(":/images/search_for_z.png"), "Probe Z");
    addSeparator();
    auto actZeroXY  = addAction(QIcon(":/images/axis_zero.png"), "Zero XY");
    auto actZeroZ   = addAction(QIcon(":/images/zero_z.png"), "Zero Z");
    addSeparator();
    auto actOrigin  = addAction(QIcon(":/images/origin.png"), "Restore Origin");
    auto actSafePos = addAction(QIcon(":/images/run.png"), "Safe Position");
    addSeparator();
    auto actNum1 = addAction(QIcon(":/images/num1.png"), "User 1");
    auto actNum2 = addAction(QIcon(":/images/num2.png"), "User 2");
    auto actNum3 = addAction(QIcon(":/images/num3.png"), "User 3");
    auto actNum4 = addAction(QIcon(":/images/num4.png"), "User 4");

    connect(actNum1, &QAction::triggered, this, &ButtonBar::cmdUser1Clicked);
    connect(actNum2, &QAction::triggered, this, &ButtonBar::cmdUser2Clicked);
    connect(actNum3, &QAction::triggered, this, &ButtonBar::cmdUser3Clicked);
    connect(actNum4, &QAction::triggered, this, &ButtonBar::cmdUser4Clicked);

    connect(actRestart, &QAction::triggered, this, &ButtonBar::cmdRestart);
    connect(actUnlock, &QAction::triggered, this, &ButtonBar::cmdUnlock);

    connect(actHome, &QAction::triggered, this, &ButtonBar::cmdHome);
    connect(actProbeZ, &QAction::triggered, this, &ButtonBar::cmdProbeZ);

    connect(actZeroXY, &QAction::triggered, this, &ButtonBar::cmdZeroXY);
    connect(actZeroZ, &QAction::triggered, this, &ButtonBar::cmdZeroZ);

    connect(actOrigin, &QAction::triggered, this, &ButtonBar::cmdOrigin);
    connect(actSafePos, &QAction::triggered, this, &ButtonBar::cmdSafePos);
}

void ButtonBar::cmdUser1Clicked()
{
    emit cmdUserClicked(1);
}

void ButtonBar::cmdUser2Clicked()
{
    emit cmdUserClicked(2);
}

void ButtonBar::cmdUser3Clicked()
{
    emit cmdUserClicked(3);
}

void ButtonBar::cmdUser4Clicked()
{
    emit cmdUserClicked(4);
}

};
