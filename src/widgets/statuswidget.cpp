#include "statuswidget.h"

namespace GUI
{

void StatusWidget::create()
{
    m_layout = new QHBoxLayout();

    m_name = new QLabel("Status");
    m_statusTxt = new QLabel("XXXXXXXXXX");

    m_layout->addWidget(m_name);
    m_layout->addWidget(m_statusTxt);

    setStatus(StatusType::UNKNOWN);

    setLayout(m_layout);
}

void StatusWidget::setColors(const QString &bkcolor, const QString &txtcolor)
{
    m_statusTxt->setStyleSheet(QString("background-color: %1; color: %2;")
        .arg(bkcolor)
        .arg(txtcolor));
}

StatusWidget::StatusType StatusWidget::statusFromString(const QString &statusStr)
{
    static const QStringList options =
    {
        "Unknown",
        "Idle",
        "Alarm",
        "Run",
        "Home",
        "Hold:0",
        "Hold:1",
        "Queue",
        "Check",
        "Door", // TODO: Update "Door" state
        "Jog"       
    };

    StatusType status{StatusType::UNKNOWN};

    auto index = options.indexOf(statusStr);
    if (index >= 0)
    {
        status = static_cast<StatusType>(index);
    }
    return status;
}

void StatusWidget::setStatus(StatusType st)
{
    if (m_status == st) return; ///< don't update widget if not necessary.

    m_status = st;
    switch(st)
    {
    case StatusType::UNKNOWN:
        m_statusTxt->setText("UNKNOWN");
        setColors("red", "white");
        break;
    case StatusType::IDLE:
        m_statusTxt->setText("IDLE");
        setColors("palette(button)", "palette(text)");
        break;
    case StatusType::ALARM:
        m_statusTxt->setText("ALARM");
        setColors("red", "white");
        break;
    case StatusType::RUN:
        m_statusTxt->setText("RUN");
        setColors("lime", "black");
        break;    
    case StatusType::HOME:
        m_statusTxt->setText("HOME");
        setColors("lime", "black");
        break;
    case StatusType::HOLD0:
        m_statusTxt->setText("HOLD 0");
        setColors("yellow", "black");
        break;        
    case StatusType::HOLD1:
        m_statusTxt->setText("HOLD 1");
        setColors("yellow", "black");
        break;                
    case StatusType::QUEUE:
        m_statusTxt->setText("QUEUE");
        setColors("yellow", "black");
        break;
    case StatusType::CHECK:
        m_statusTxt->setText("QUEUE");
        setColors("palette(button)", "palette(text)");
        break;
    case StatusType::DOOR:        
        m_statusTxt->setText("DOOR");
        setColors("red", "white");
        break;                
    case StatusType::JOG:        
        m_statusTxt->setText("DOOR");
        setColors("lime", "black");
        break;                   
    }
}

};