#pragma once 

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QBoxLayout>

#include "slider.h"

class SliderBox : public QWidget
{
    Q_OBJECT
public:
    explicit SliderBox(QWidget *parent = 0);
    ~SliderBox();

    int value();
    void setValue(int value);

    int currentValue();
    void setCurrentValue(int value);

    int sliderPosition();
    void setSliderPosition(int position);

    bool isCheckable() const;
    void setCheckable(bool checkable);

    bool isChecked();
    void setChecked(bool checked);

    int ratio() const;
    void setRatio(int ratio);

    int minimum() const;
    void setMinimum(int minimum);

    int maximum() const;
    void setMaximum(int maximum);

    QString suffix();
    void setSuffix(QString suffix);

    QString title();
    void setTitle(QString title);

signals:
    void valueUserChanged();
    void valueChanged();
    void toggled(bool checked);

private slots:
    void onTimerValueChanged();

    void onTxtValueEditingFinished();
    void onSliderActionTriggered(int action);
    void onSliderValueChanged(int value);

private:
    void createWidget();

    QVBoxLayout *m_mainLayout{nullptr};
    QHBoxLayout *m_horizontalLayout{nullptr};
    Slider      *m_slider{nullptr};
    QSpinBox    *m_txtValue{nullptr};
    QLabel      *m_title{nullptr};
    QCheckBox   *m_checkbox{nullptr};

    bool m_isCheckable;
    int m_ratio;

    int m_currentValue;
    int m_minimum, m_maximum;

    QTimer m_timerValueChanged;
};

