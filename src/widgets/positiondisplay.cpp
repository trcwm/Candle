#include "positiondisplay.h"
#include <QFont>

namespace GUI
{

PositionDisplay::PositionDisplay(const QString &name, QWidget *parent) : QWidget(parent)
{
    create();
    setName(name);
}

PositionDisplay::PositionDisplay(QWidget *parent) : QWidget(parent)
{
    create();
}

void PositionDisplay::create()
{
    QFont font("Monospace");
    font.setStyleHint(QFont::Monospace);

    m_layout = new QHBoxLayout;
    m_name   = new QLabel("WORK");
    m_x      = new QLabel("0000");
    m_y      = new QLabel("0000");
    m_z      = new QLabel("0000");

    m_x->setFont(font);
    m_y->setFont(font);
    m_z->setFont(font);

    m_layout->addWidget(m_name);
    m_layout->addWidget(m_x);
    m_layout->addWidget(m_y);
    m_layout->addWidget(m_z);

    setLayout(m_layout);
}

void PositionDisplay::setPosition(const QVector3D &pos)
{
    m_pos = pos;

    auto xx = static_cast<int>(pos.x());
    auto yy = static_cast<int>(pos.y());
    auto zz = static_cast<int>(pos.z());

    m_x->setText(QString::asprintf("%4d", xx));
    m_y->setText(QString::asprintf("%4d", yy));
    m_z->setText(QString::asprintf("%4d", zz));    
}

void PositionDisplay::setPosition(float x, float y, float z)
{
    setPosition(QVector3D(x,y,z));
}

};
