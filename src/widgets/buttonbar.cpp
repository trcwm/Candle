#include "buttonbar.h"

namespace GUI
{

ButtonBar::ButtonBar(QWidget *parent) : QToolBar(parent)
{
    createWidget();
}

void ButtonBar::createWidget()
{
    m_actRestart = addAction(QIcon(":/images/restart.png"), "Restart");
    m_actUnlock  = addAction(QIcon(":/images/unlock.png"), "Unlock");
    addSeparator();
    m_actHome    = addAction(QIcon(":/images/search_for_home2.png"), "Home");
    m_actProbeZ  = addAction(QIcon(":/images/search_for_z.png"), "Probe Z");
    addSeparator();
    m_actZeroXY  = addAction(QIcon(":/images/axis_zero.png"), "Zero XY");
    m_actZeroZ   = addAction(QIcon(":/images/zero_z.png"), "Zero Z");
    addSeparator();
    m_actOrigin  = addAction(QIcon(":/images/origin.png"), "Restore Origin");
    m_actSafePos = addAction(QIcon(":/images/run.png"), "Safe Position");
    addSeparator();
    m_actNum1 = addAction(QIcon(":/images/num1.png"), "User 1");
    m_actNum2 = addAction(QIcon(":/images/num2.png"), "User 2");
    m_actNum3 = addAction(QIcon(":/images/num3.png"), "User 3");
    m_actNum4 = addAction(QIcon(":/images/num4.png"), "User 4");

    connect(m_actNum1, &QAction::triggered, this, &ButtonBar::cmdUser1Clicked);
    connect(m_actNum2, &QAction::triggered, this, &ButtonBar::cmdUser2Clicked);
    connect(m_actNum3, &QAction::triggered, this, &ButtonBar::cmdUser3Clicked);
    connect(m_actNum4, &QAction::triggered, this, &ButtonBar::cmdUser4Clicked);

    connect(m_actRestart, &QAction::triggered, this, &ButtonBar::cmdRestart);
    connect(m_actUnlock, &QAction::triggered, this, &ButtonBar::cmdUnlock);

    connect(m_actHome, &QAction::triggered, this, &ButtonBar::cmdHome);
    connect(m_actProbeZ, &QAction::triggered, this, &ButtonBar::cmdProbeZ);

    connect(m_actZeroXY, &QAction::triggered, this, &ButtonBar::cmdZeroXY);
    connect(m_actZeroZ, &QAction::triggered, this, &ButtonBar::cmdZeroZ);

    connect(m_actOrigin, &QAction::triggered, this, &ButtonBar::cmdOrigin);
    connect(m_actSafePos, &QAction::triggered, this, &ButtonBar::cmdSafePos);
}

void ButtonBar::enableUserButtons(bool enabled)
{
    m_actNum1->setEnabled(enabled);
    m_actNum2->setEnabled(enabled);
    m_actNum3->setEnabled(enabled);
    m_actNum4->setEnabled(enabled);
}

void ButtonBar::enableControlButtons(bool enabled)
{
    m_actHome->setEnabled(enabled);
    m_actProbeZ->setEnabled(enabled);
    m_actZeroXY->setEnabled(enabled);
    m_actZeroZ->setEnabled(enabled);
    m_actOrigin->setEnabled(enabled);
    m_actUnlock->setEnabled(enabled);
    m_actSafePos->setEnabled(enabled);
}

void ButtonBar::enablePositionButtons(bool enabled)
{
    m_actOrigin->setEnabled(enabled);
    m_actSafePos->setEnabled(enabled);
    m_actZeroXY->setEnabled(enabled);
    m_actZeroZ->setEnabled(enabled);
}

void ButtonBar::setOriginTooltip(const QString &txt)
{
    m_actOrigin->setToolTip(txt);
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
