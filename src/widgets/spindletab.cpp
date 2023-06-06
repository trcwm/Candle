#include "spindletab.h"
#include <QIcon>
#include <QSpacerItem>

namespace GUI
{

SpindleTab::SpindleTab(QWidget *parent) : QWidget(parent)
{
    createWidget();
}

void SpindleTab::createWidget()
{
    m_layout = new QVBoxLayout();

    auto toolLayout = new QHBoxLayout();

    m_sliderBox = new SliderBox();
    m_button = new StyledToolButton();
    m_button->setIcon(QIcon(":/images/cutter.png"));
    m_button->setCheckable(true);
    m_button->setEnabled(true);
        
    m_sliderBox->setTitle("Speed:");
    m_sliderBox->setCheckable(false);
    m_sliderBox->setChecked(true);
    m_sliderBox->setRatio(100);

    toolLayout->addWidget(m_sliderBox);
    toolLayout->addWidget(m_button);

    auto spacer = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_layout->addLayout(toolLayout);
    m_layout->addItem(spacer);

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
