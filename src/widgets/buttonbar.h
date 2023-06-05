#pragma once

#include <QWidget>
#include <QToolBar>

namespace GUI
{

class ButtonBar : public QToolBar
{
    Q_OBJECT
public:
    ButtonBar(QWidget *parent = nullptr);

signals:
    void cmdUserClicked(int id);
    void cmdHome();
    void cmdProbeZ();
    void cmdOrigin();
    void cmdSafePos();
    void cmdZeroXY();
    void cmdZeroZ();
    void cmdUnlock();
    void cmdRestart();
    
private slots:
    void cmdUser1Clicked();
    void cmdUser2Clicked();
    void cmdUser3Clicked();
    void cmdUser4Clicked();

protected:
    void createWidget();
};

};