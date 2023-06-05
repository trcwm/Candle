#include "jogwidget.h"
#include <QIcon>
#include <QLabel>
#include <QSpacerItem>
#include <QValidator>

namespace GUI
{

JogWidget::JogWidget(QWidget *parent) : QWidget(parent)
{
    createWidget();
}

void JogWidget::createWidget()
{
    m_layout = new QGridLayout();

    m_upX = new StyledToolButton();
    m_upX->setMinimumSize(36,36);
    m_upX->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_upX->setIcon(QIcon(":/images/1401562173_chevron-right.png"));

    m_downX = new StyledToolButton();
    m_downX->setMinimumSize(36,36);
    m_downX->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_downX->setIcon(QIcon(":/images/1401561986_chevron-left.png"));

    m_upY = new StyledToolButton();
    m_upY->setMinimumSize(36,36);
    m_upY->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_upY->setIcon(QIcon(":/images/1401562173_chevron-up.png"));

    m_downY = new StyledToolButton();
    m_downY->setMinimumSize(36,36);
    m_downY->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_downY->setIcon(QIcon(":/images/1401562173_chevron-down.png"));

    m_upZ = new StyledToolButton();
    m_upZ->setMinimumSize(36,36);
    m_upZ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_upZ->setIcon(QIcon(":/images/1401562699_icon-arrow-up-b.png"));

    m_downZ = new StyledToolButton();
    m_downZ->setMinimumSize(36,36);
    m_downZ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_downZ->setIcon(QIcon(":/images/1401562699_icon-arrow-down-b.png"));

    m_stop = new StyledToolButton();
    m_stop->setMinimumSize(36,36);
    m_stop->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_stop->setIcon(QIcon(":/images/brake.png"));

    m_jogStep = new ComboBoxKey();
    m_jogFeed = new ComboBoxKey();

    m_layout->addWidget(m_upY, 0, 1);
    m_layout->addWidget(m_downY, 2, 1);
    m_layout->addWidget(m_upX, 1, 2);
    m_layout->addWidget(m_downX, 1, 0);
    m_layout->addWidget(m_stop, 1, 1);

    m_layout->addItem(new QSpacerItem(0,0, QSizePolicy::Expanding, QSizePolicy::Expanding), 1,3);

    m_layout->addWidget(m_upZ, 0, 4);
    m_layout->addWidget(m_downZ, 2, 4);
    
    m_layout->addWidget(new QLabel("Step"), 3, 0);
    m_layout->addWidget(new QLabel("Feed"), 4, 0);
    m_layout->addWidget(m_jogStep, 3, 1, 1, 4);
    m_layout->addWidget(m_jogFeed, 4, 1, 1, 4);

    setLayout(m_layout);

    m_jogStep->setEditable(true);
    m_jogStep->setCurrentText("Continuous");
    m_jogFeed->setEditable(true);
    m_jogFeed->setCurrentText("10");

    m_jogStep->setValidator(new QDoubleValidator(0, 10000, 2));
    m_jogFeed->setValidator(new QIntValidator(0, 100000));

    connect(m_stop, &StyledToolButton::pressed, this, &JogWidget::onStopPressed);

    connect(m_upX, &StyledToolButton::pressed, this, &JogWidget::onXUp);
    connect(m_upY, &StyledToolButton::pressed, this, &JogWidget::onYUp);
    connect(m_upZ, &StyledToolButton::pressed, this, &JogWidget::onZUp);
    connect(m_upX, &StyledToolButton::released, this, &JogWidget::onXUp);
    connect(m_upY, &StyledToolButton::released, this, &JogWidget::onYUp);
    connect(m_upZ, &StyledToolButton::released, this, &JogWidget::onZUp);

    connect(m_downX, &StyledToolButton::pressed, this, &JogWidget::onXDown);
    connect(m_downY, &StyledToolButton::pressed, this, &JogWidget::onYDown);
    connect(m_downZ, &StyledToolButton::pressed, this, &JogWidget::onZDown);
    connect(m_downX, &StyledToolButton::released, this, &JogWidget::onXDown);
    connect(m_downY, &StyledToolButton::released, this, &JogWidget::onYDown);
    connect(m_downZ, &StyledToolButton::released, this, &JogWidget::onZDown);
}

void JogWidget::setStepItems(const QStringList &list)
{
    m_jogStep->items().clear();
    m_jogStep->items() = list;
    m_jogStep->setCurrentIndex(0);
}

void JogWidget::setFeedItems(const QStringList &list)
{
    m_jogFeed->items().clear();
    m_jogFeed->items() = list;
    m_jogFeed->setCurrentIndex(0);
}

QStringList JogWidget::getStepItems()
{
    return m_jogStep->items();
}

QStringList JogWidget::getFeedItems()
{
    return m_jogFeed->items();
}

void JogWidget::onStopPressed()
{
    m_jogVector = QVector3D{0,0,0};
    emit stopClicked();
    emit jogVectorChanged();    
}

void JogWidget::onXUp()
{
    bool state = m_upX->isDown();

    if (state)
    {
        m_jogVector += QVector3D{1,0,0};
    }
    else
    {
        m_jogVector -= QVector3D{1,0,0};
    }
    emit jogVectorChanged();
}

void JogWidget::onYUp()
{
    bool state = m_upY->isDown();
    if (state)
    {
        m_jogVector += QVector3D{0,1,0};
    }
    else
    {
        m_jogVector -= QVector3D{0,1,0};
    }
    emit jogVectorChanged();
}

void JogWidget::onZUp()
{
    bool state = m_upZ->isDown();
    if (state)
    {
        m_jogVector += QVector3D{0,0,1};
    }
    else
    {
        m_jogVector -= QVector3D{0,0,1};
    }
    emit jogVectorChanged();
}

void JogWidget::onXDown()
{
    bool state = m_downX->isDown();
    if (state)
    {
        m_jogVector += QVector3D{-1,0,0};
    }
    else
    {
        m_jogVector -= QVector3D{-1,0,0};
    }
    emit jogVectorChanged();
}

void JogWidget::onYDown()
{
    bool state = m_downY->isDown();
    if (state)
    {
        m_jogVector += QVector3D{0,-1,0};
    }
    else
    {
        m_jogVector -= QVector3D{0,-1,0};
    }
    emit jogVectorChanged();
}

void JogWidget::onZDown()
{
    bool state = m_downZ->isDown();
    if (state)
    {
        m_jogVector += QVector3D{0,0,-1};
    }
    else
    {
        m_jogVector -= QVector3D{0,0,-1};
    }
    emit jogVectorChanged();
}

};


