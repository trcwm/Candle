#include "spindletab.h"
#include <QIcon>

namespace GUI
{

SpindleTab::SpindleTab(QWidget *parent) : QWidget(parent)
{
    createWidget();
}

void SpindleTab::createWidget()
{
    m_layout = new QHBoxLayout();

    m_sliderBox = new SliderBox();
    m_button = new StyledToolButton();
    m_button->setIcon(QIcon(":/images/cutter.png"));
    //m_button->setIcon(QIcon(":/images/cubeTop.png"));
    
    m_sliderBox->setTitle("Speed:");
    m_sliderBox->setCheckable(false);
    m_sliderBox->setChecked(true);
    m_sliderBox->setRatio(100);

    m_layout->addWidget(m_sliderBox,1);
    m_layout->addWidget(m_button);

    setLayout(m_layout);

    connect(m_button, &StyledToolButton::clicked, this, &SpindleTab::clicked);
    connect(m_button, &StyledToolButton::toggled, this, &SpindleTab::toggled);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

void SpindleTab::setValue(int spindleSpeed)
{
    m_sliderBox->setValue(spindleSpeed);
}

int  SpindleTab::value() const
{
    return m_sliderBox->value();
}

void SpindleTab::setMaxSpindleSpeed(int max)
{
    m_sliderBox->setMaximum(max);
}

void SpindleTab::setMinSpindleSpeed(int min)
{
    m_sliderBox->setMinimum(min);
}

};
