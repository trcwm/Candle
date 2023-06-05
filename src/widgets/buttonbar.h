#pragma once

#include <QWidget>
#include <QAction>
#include <QToolBar>

namespace GUI
{

class ButtonBar : public QToolBar
{
    Q_OBJECT
public:
    ButtonBar(QWidget *parent = nullptr);

    void setOriginTooltip(const QString &txt);
    void enableControlButtons(bool enabled = true);
    void enableUserButtons(bool enabled = true);
    void enablePositionButtons(bool enabled = true);
    
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

    QAction* m_actRestart{nullptr};
    QAction* m_actUnlock{nullptr}; 
    QAction* m_actHome{nullptr};   
    QAction* m_actProbeZ{nullptr}; 
    QAction* m_actZeroXY{nullptr}; 
    QAction* m_actZeroZ{nullptr};  
    QAction* m_actOrigin{nullptr}; 
    QAction* m_actSafePos{nullptr};
    QAction* m_actNum1{nullptr};   
    QAction* m_actNum2{nullptr};   
    QAction* m_actNum3{nullptr};   
    QAction* m_actNum4{nullptr};   
};

};