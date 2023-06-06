#pragma once
#include <QWidget>
#include <QBoxLayout>

#include "sliderbox.h"

namespace GUI
{

class OverrideTab : public QWidget
{
    Q_OBJECT
public:
     OverrideTab(QWidget *parent = nullptr);

    SliderBox* feed()
    {
        return m_feed;
    }

    SliderBox* rapid()
    {
        return m_rapid;
    }

    SliderBox* spindle()
    {
        return m_spindle;
    }

protected:
    void createWidget();

    QVBoxLayout *m_layout{nullptr};
    
    SliderBox *m_feed{nullptr};
    SliderBox *m_rapid{nullptr};
    SliderBox *m_spindle{nullptr};
};

};
