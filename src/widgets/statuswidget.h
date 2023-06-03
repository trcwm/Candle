#pragma once
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

namespace GUI
{

class StatusWidget : public QWidget
{
public:
    StatusWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        create();
    }

    enum class StatusType : int
    {
        UNKNOWN = 0,
        IDLE,
        ALARM,
        RUN,
        HOME,
        HOLD0,
        HOLD1,
        QUEUE,
        CHECK,
        DOOR,
        JOG
    };

    static StatusType statusFromString(const QString &statusStr);

    void create();
    
    void reset()
    {
        setStatus(StatusType::UNKNOWN);
    }

    void setStatus(StatusType stat);

    StatusType getStatus() const noexcept
    {
        return m_status;
    }

    void setColors(const QString &bkcolor, const QString &txtcolor);

protected:
    StatusType   m_status{StatusType::UNKNOWN};
    QLabel      *m_name{nullptr};
    QLabel      *m_statusTxt{nullptr};
    QHBoxLayout *m_layout{nullptr};
};

};