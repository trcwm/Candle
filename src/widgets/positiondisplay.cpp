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
    QFont myFont("Monospace");
    myFont.setStyleHint(QFont::Monospace);

    m_layout = new QHBoxLayout();
    m_name   = new QLabel("");
    m_x      = new QLabel("");
    m_y      = new QLabel("");
    m_z      = new QLabel("");

    QFontMetricsF fm{myFont};
    auto brect = fm.boundingRect("Machine:");
    auto minHeight = brect.size().toSize().grownBy({2,2,2,2}).height();
    m_name->setFixedSize(brect.size().toSize().grownBy({2,2,2,2}));

    //m_x->setReadOnly(true);
    //m_y->setReadOnly(true);
    //m_z->setReadOnly(true);

    m_x->setStyleSheet("background-color: white; border-style: solid; border-color: rgb(100,100,100); border-width: 1px;");
    m_y->setStyleSheet("background-color: white; border-style: solid; border-color: rgb(100,100,100); border-width: 1px;");
    m_z->setStyleSheet("background-color: white; border-style: solid; border-color: rgb(100,100,100); border-width: 1px;");

    m_x->setFont(myFont);
    m_y->setFont(myFont);
    m_z->setFont(myFont);

    m_x->setMaximumHeight(minHeight);
    m_y->setMaximumHeight(minHeight);
    m_z->setMaximumHeight(minHeight);

    m_layout->addWidget(m_name);
    m_layout->addWidget(m_x);
    m_layout->addWidget(m_y);
    m_layout->addWidget(m_z);

    setPosition(0.0f,0.0f,0.0f);

    setLayout(m_layout);

#if 1
    m_x->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_y->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_z->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
#endif    
}

void PositionDisplay::setPosition(const QVector3D &pos)
{
    m_pos = pos;
    m_x->setText(QString::asprintf("X %07.2f", pos.x()));
    m_y->setText(QString::asprintf("Y %07.2f", pos.y()));
    m_z->setText(QString::asprintf("Z %07.2f", pos.z()));
}

void PositionDisplay::setPosition(float x, float y, float z)
{
    setPosition(QVector3D(x,y,z));
}

};
