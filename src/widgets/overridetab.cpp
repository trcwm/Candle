#include "overridetab.h"

namespace GUI
{

OverrideTab::OverrideTab(QWidget *parent) : QWidget(parent)
{
    createWidget();
}

void OverrideTab::createWidget()
{
    m_layout = new QVBoxLayout();

    m_feed  = new SliderBox();
    m_rapid = new SliderBox();
    m_spindle  = new SliderBox();

    m_layout->addWidget(m_feed);
    m_layout->addWidget(m_rapid);
    m_layout->addWidget(m_spindle);

    setLayout(m_layout);
}

};