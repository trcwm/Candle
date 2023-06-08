// This file is a part of "Candle" application.
// Copyright 2015-2016 Hayrullin Denis Ravilevich

#include "styledtoolbutton.h"
#include <QDebug>
#include <QFontMetricsF>
#include <QEvent>

StyledToolButton::StyledToolButton(QWidget *parent) : QAbstractButton(parent)
{
    m_hovered = false;
    m_backColor = palette().color(QPalette::Button);
    m_foreColor = palette().color(QPalette::ButtonText);
    m_highlightColor = QColor(127, 211, 255).darker(120);
}

bool StyledToolButton::isHover()
{
    return m_hovered;
}

void StyledToolButton::enterEvent(QEvent *e)
{
    Q_UNUSED(e)

    m_hovered = true;
}

void StyledToolButton::leaveEvent(QEvent *e)
{
    Q_UNUSED(e)

    m_hovered = false;
}

void StyledToolButton::setText(const QString &txt)
{
    QAbstractButton::setText(txt);
    setMinimumSize(calcMinimumSize());
}

void StyledToolButton::setIcon(const QIcon &icon)
{
    QAbstractButton::setIcon(icon);

    setMinimumSize(calcMinimumSize());
}

QSize StyledToolButton::calcMinimumSize() const
{
    // QRect innerRect(m_borderWidth, m_borderWidth, width() - m_borderWidth * 2, height() - m_borderWidth * 2);

    QSize recommendedSize{0,0};
    if (icon().isNull())
    {   
        // button has text only
        QFontMetricsF fm{font()};
        auto brect = fm.boundingRect(text());
        recommendedSize = brect.size().toSize();
    }
    else
    {
        // button has icon only
        recommendedSize = icon().actualSize(iconSize());        
        recommendedSize = recommendedSize * 2;
    }

    recommendedSize += QSize{m_borderWidth*2, m_borderWidth*2};

    return recommendedSize;
}

void StyledToolButton::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)


    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);

    // Highlight
    QPen highlightPen;

    if ((!this->isEnabled() && !this->isChecked()) || (!this->isDown() && !this->isChecked() && !this->isHover()))
    {
        highlightPen.setColor(Qt::white);
    }
    else if (this->isDown() || this->isChecked())
    {
        highlightPen.setColor(m_highlightColor);
    }
    else if (this->isHover())
    {
        highlightPen.setColor(m_highlightColor.lighter(120));
    }

    highlightPen.setWidth(2);
    painter.setPen(highlightPen);
    painter.drawRoundedRect(1, 1, width() - 2, height() - 2, m_borderRadius - 1, m_borderRadius - 1);

    // Border
    QPen pen(isEnabled() ? palette().color(QPalette::Shadow) : palette().color(QPalette::Mid));

    if ((isDown() || isChecked()) && isEnabled())
        pen.setColor(Qt::black);

    pen.setWidth(2);
    pen.setCapStyle(Qt::SquareCap);
    painter.setPen(pen);

    painter.drawLine(m_borderRadius, 0, width() - m_borderRadius, 0);
    painter.drawLine(m_borderRadius, height(), width() - m_borderRadius, height());
    painter.drawLine(0, m_borderRadius, 0, height() - m_borderRadius);
    painter.drawLine(width(), m_borderRadius, width(), height() - m_borderRadius);

    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawArc(0, 0, m_borderRadius * 2, m_borderRadius * 2, 90 * 16, 90 * 16);
    painter.drawArc(width() - m_borderRadius * 2, 0, m_borderRadius * 2, m_borderRadius * 2, 0 * 16, 90 * 16);
    painter.drawArc(0, height() - m_borderRadius * 2, m_borderRadius * 2, m_borderRadius * 2, 180 * 16, 90 * 16);
    painter.drawArc(width() - m_borderRadius * 2, height() - m_borderRadius * 2, m_borderRadius * 2, m_borderRadius * 2, 270 * 16, 90 * 16);

    // Background border
    QLinearGradient backGradient(width() / 2, height() / 2, width() / 2, height());
    backGradient.setColorAt(0, isEnabled() ? m_backColor : palette().color(QPalette::Button));
    backGradient.setColorAt(1, isEnabled() ? m_backColor.darker(130) : palette().color(QPalette::Button).darker(130));
    QBrush backBrush(backGradient);
    painter.setBrush(backBrush);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(m_borderWidth - 1, m_borderWidth - 1, width() - m_borderWidth * 2 + 2, height() - m_borderWidth * 2 + 2, 2, 2);

    // Background
    painter.setBrush(this->isEnabled() ? m_backColor : palette().color(QPalette::Button));
    painter.setPen(Qt::NoPen);
    painter.drawRect(m_borderWidth, m_borderWidth, width() - m_borderWidth * 2, height() - m_borderWidth * 2);

    // Icon/text rect
    QRect innerRect(m_borderWidth, m_borderWidth, width() - m_borderWidth * 2, height() - m_borderWidth * 2);
    if (this->isDown() || this->isChecked())
    {
        innerRect.setLeft(innerRect.left() + 2);
        innerRect.setTop(innerRect.top() + 2);
    }

    // Icon
    if (!icon().isNull())
    {
        QSize _iconSize = icon().actualSize(iconSize());
        painter.drawPixmap(QRect(innerRect.x() + (innerRect.width() - _iconSize.width()) / 2,
                                 innerRect.y() + (innerRect.height() - _iconSize.height()) / 2,
                                 _iconSize.width(), _iconSize.height()),
                           icon().pixmap(_iconSize, isEnabled() ? QIcon::Normal : QIcon::Disabled));
    }
    else
    {
        // Text
        painter.setPen(isEnabled() ? m_foreColor : palette().color(QPalette::Mid));
        painter.drawText(innerRect, Qt::AlignCenter, text());
    }
}

QColor StyledToolButton::highlightColor() const
{
    return m_highlightColor;
}

void StyledToolButton::setHighlightColor(const QColor &highlightColor)
{
    m_highlightColor = highlightColor;
}

QColor StyledToolButton::foreColor() const
{
    return m_foreColor;
}

void StyledToolButton::setForeColor(const QColor &foreColor)
{
    m_foreColor = foreColor;
}

QColor StyledToolButton::backColor() const
{
    return m_backColor;
}

void StyledToolButton::setBackColor(const QColor &backColor)
{
    m_backColor = backColor;
}
