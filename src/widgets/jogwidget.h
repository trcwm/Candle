#pragma once
#include <QWidget>
#include <QVector3D>
#include <QGridLayout>
#include "styledtoolbutton.h"
#include "comboboxkey.h"

namespace GUI
{
    
class JogWidget : public QWidget
{
    Q_OBJECT
public:
    JogWidget(QWidget *parent = nullptr);
    virtual ~JogWidget() = default;

    constexpr QVector3D getJogVector() const noexcept
    {
        return m_jogVector;
    }

    /** returns the step size in mm, or 0 is continuous */
    float getStep() const
    {        
        return m_jogStep->currentText().toDouble();
    }

    /** returns the feed rate in mm/minute */
    float getFeed() const
    {        
        return m_jogFeed->currentText().toDouble();
    }

    void setStepItems(const QStringList &list);
    void setFeedItems(const QStringList &list);
    QStringList getStepItems();
    QStringList getFeedItems();

signals:
    void jogVectorChanged();
    void stopClicked();

private slots:
    void onStopPressed();
    void onXUp();
    void onYUp();
    void onZUp();
    void onXDown();
    void onYDown();
    void onZDown();

protected:
    void createWidget();

    QGridLayout *m_layout{nullptr};

    StyledToolButton *m_upX{nullptr};
    StyledToolButton *m_downX{nullptr};
    StyledToolButton *m_upY{nullptr};
    StyledToolButton *m_downY{nullptr};    
    StyledToolButton *m_upZ{nullptr};
    StyledToolButton *m_downZ{nullptr};        
    StyledToolButton *m_stop{nullptr};

    ComboBoxKey *m_jogStep{nullptr};
    ComboBoxKey *m_jogFeed{nullptr};

    QVector3D m_jogVector{0,0,0};
};

};
