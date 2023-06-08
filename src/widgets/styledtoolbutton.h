// This file is a part of "Candle" application.
// Copyright 2015-2016 Hayrullin Denis Ravilevich

#ifndef STYLEDTOOLBUTTON_H
#define STYLEDTOOLBUTTON_H

#include <QWidget>
#include <QAbstractButton>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionFrame>

class StyledToolButton : public QAbstractButton
{
public:
    explicit StyledToolButton(QWidget *parent = 0);

    bool isHover();

    QColor backColor() const;
    void setBackColor(const QColor &backColor);

    QColor foreColor() const;
    void setForeColor(const QColor &foreColor);

    QColor highlightColor() const;
    void setHighlightColor(const QColor &highlightColor);

    void setText(const QString &txt);
    void setIcon(const QIcon &icon);

protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

private:
    void paintEvent(QPaintEvent *e);
    QSize calcMinimumSize() const;

    bool m_hovered;
    QColor m_backColor;
    QColor m_foreColor;
    QColor m_highlightColor;

    constexpr static int m_borderWidth = 4;
    constexpr static int m_borderRadius = 5;
};

#endif // STYLEDTOOLBUTTON_H
