
#include "sliderbox.h"

SliderBox::SliderBox(QWidget *parent) :
    QWidget(parent)
{
    createWidget();

    setCheckable(true);
    setRatio(1);
    setMinimum(0);
    setMaximum(10000);
    setValue(0);
    setCurrentValue(0);

    connect(&m_timerValueChanged, SIGNAL(timeout()), this, SLOT(onTimerValueChanged()));
    m_timerValueChanged.setInterval(250);
}

SliderBox::~SliderBox()
{
}

void SliderBox::createWidget()
{
    m_mainLayout = new QVBoxLayout();
    m_horizontalLayout = new QHBoxLayout();

    m_checkbox = new QCheckBox();
    m_horizontalLayout->addWidget(m_checkbox);

    m_title = new QLabel("");
    m_horizontalLayout->addWidget(m_title);

    m_txtValue = new QSpinBox();
    m_horizontalLayout->addWidget(m_txtValue, 1);

    m_slider = new Slider();
    m_slider->setOrientation(Qt::Horizontal);

    m_mainLayout->addLayout(m_horizontalLayout);
    m_mainLayout->addWidget(m_slider);
    
    setLayout(m_mainLayout);

    connect(m_slider, &Slider::actionTriggered, this, &SliderBox::onSliderActionTriggered);
    connect(m_slider, &Slider::valueChanged, this, &SliderBox::onSliderValueChanged);
    connect(m_txtValue, &QSpinBox::editingFinished, this, &SliderBox::onTxtValueEditingFinished);
    connect(m_checkbox, &QCheckBox::toggled, this, &SliderBox::toggled);
}


int SliderBox::value()
{
    return m_txtValue->value();
}

void SliderBox::setValue(int value)
{
    m_txtValue->setValue(value);
    m_slider->setValue(value / m_ratio);
}

int SliderBox::currentValue()
{
    return m_currentValue;
}

void SliderBox::setCurrentValue(int value)
{
    m_currentValue = value;

    m_slider->setCurrentValue(value / m_ratio);
    m_txtValue->setStyleSheet(value == m_txtValue->value() || !isChecked() ? "color: palette(text);" : "color: red;");
}

int SliderBox::sliderPosition()
{
    return m_slider->sliderPosition();
}

void SliderBox::setSliderPosition(int position)
{
    m_slider->setSliderPosition(position);
}

bool SliderBox::isCheckable() const
{
    return m_isCheckable;
}

void SliderBox::setCheckable(bool checkable)
{
    m_isCheckable = checkable;

    m_checkbox->setVisible(checkable);
}

bool SliderBox::isChecked()
{
    return m_checkbox->isChecked();
}

void SliderBox::setChecked(bool checked)
{
    m_checkbox->setChecked(checked);
}

int SliderBox::ratio() const
{
    return m_ratio;
}

void SliderBox::setRatio(int ratio)
{
    m_ratio = ratio;
}

void SliderBox::onTxtValueEditingFinished()
{
    m_slider->setValue(m_txtValue->value() / ratio());
    emit valueUserChanged();
}

void SliderBox::onSliderActionTriggered(int action)
{
    Q_UNUSED(action);

    m_txtValue->setValue(m_slider->sliderPosition() * ratio());
    emit valueUserChanged();
}

void SliderBox::onSliderValueChanged(int value)
{
    Q_UNUSED(value);

    if (isChecked()) 
    {
        m_txtValue->setStyleSheet("color: red;");
        m_timerValueChanged.start();
    }
}

void SliderBox::onTimerValueChanged()
{
    m_timerValueChanged.stop();
    emit valueChanged();
}

int SliderBox::maximum() const
{
    return m_maximum;
}

void SliderBox::setMaximum(int maximum)
{
    m_maximum = maximum;

    m_txtValue->setMaximum(maximum);
    m_slider->setMaximum(maximum / m_ratio);
}

QString SliderBox::suffix()
{
    return m_txtValue->suffix();
}

void SliderBox::setSuffix(QString suffix)
{
    m_txtValue->setSuffix(suffix);
}

QString SliderBox::title()
{
    return m_title->text();
}

void SliderBox::setTitle(QString title)
{
    m_title->setText(title);
}

int SliderBox::minimum() const
{
    return m_minimum;
}

void SliderBox::setMinimum(int minimum)
{
    m_minimum = minimum;

    m_txtValue->setMinimum(minimum);
    m_slider->setMinimum(minimum / m_ratio);
}

void SliderBox::onCheckboxToggled(bool checked)
{
    emit toggled(checked);
}
