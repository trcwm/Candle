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

protected:
    void createWidget();
};

};