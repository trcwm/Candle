#include "positiondisplay.h"
#include <QFont>

namespace GUI
{

PositionDisplay::PositionDisplay(const QString &name, QWidget *parent) : QWidget(parent)
{
    createWidgets();
    setName(name);
}

PositionDisplay::PositionDisplay(QWidget *parent) : QWidget(parent)
{
    createWidgets();
}

void PositionDisplay::createWidgets()
{
    //QWidget::setStyleSheet("border-style: solid; border-color: rgb(100,100,100); border-width: 1px;");

    QFont font("Monospace");
    font.setStyleHint(QFont::Monospace);

    m_layout = new QHBoxLayout();
    m_name   = new QLabel("");
    m_x      = new QLabel("");
    m_y      = new QLabel("");
    m_z      = new QLabel("");

    m_x->setStyleSheet("background-color: white; border-style: solid; border-color: rgb(100,100,100); border-width: 1px;");
    m_y->setStyleSheet("background-color: white; border-style: solid; border-color: rgb(100,100,100); border-width: 1px;");
    m_z->setStyleSheet("background-color: white; border-style: solid; border-color: rgb(100,100,100); border-width: 1px;");

    m_x->setFont(font);
    m_y->setFont(font);
    m_z->setFont(font);

    m_layout->addWidget(m_name);
    m_layout->addWidget(m_x);
    m_layout->addWidget(m_y);
    m_layout->addWidget(m_z);

    setPosition(0.0f,0.0f,0.0f);

    setLayout(m_layout);
}

void PositionDisplay::setPosition(const QVector3D &pos)
{
    m_pos = pos;
    m_x->setText(QString::asprintf("X %06.2f", pos.x()));
    m_y->setText(QString::asprintf("Y %06.2f", pos.y()));
    m_z->setText(QString::asprintf("Z %06.2f", pos.z()));
}

void PositionDisplay::setPosition(float x, float y, float z)
{
    setPosition(QVector3D(x,y,z));
}

};
