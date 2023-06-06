#pragma once
#include <QWidget>
#include <QBoxLayout>
#include "sliderbox.h"
#include "styledtoolbutton.h"

namespace GUI
{

class SpindleTab : public QWidget
{
    Q_OBJECT
public:
    SpindleTab(QWidget *parent = nullptr);

    auto spindleValue() const 
    {
        return m_sliderBox->value();
    }

    SliderBox* slider()
    {
        return m_sliderBox;
    }

    void setValue(int spindleSpeed);
    int  value() const;

    void setMaxSpindleSpeed(int max);
    void setMinSpindleSpeed(int min);

    bool isChecked() const
    {
        return m_button->isChecked();
    }

    void setChecked(bool checked)
    {
        m_button->setChecked(checked);
    }

    void incrementSlider()
    {
        m_sliderBox->setSliderPosition(m_sliderBox->sliderPosition() + 1);
    }

    void decrementSlider()
    {
        m_sliderBox->setSliderPosition(m_sliderBox->sliderPosition() - 1);
    }

    void enableButton(bool enabled = true)
    {
        m_button->setEnabled(enabled);
    }

signals:
    void clicked(bool checked);
    void toggled(bool checked);

protected:
    void createWidget();

    SliderBox   *m_sliderBox{nullptr};
    StyledToolButton *m_button{nullptr};
    QHBoxLayout *m_layout{nullptr};
};

};