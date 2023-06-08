#pragma once
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QBoxLayout>
#include <QString>
#include <QVector3D>

namespace GUI
{

/** widget to display x,y and z position of the CNC */
class PositionDisplay : public QWidget
{
    Q_OBJECT
public:
    PositionDisplay(QWidget *parent = nullptr);
    PositionDisplay(const QString &name, QWidget *parent = nullptr);

    /** set the position in mm */
    void setPosition(const QVector3D &pos);
    void setPosition(float x, float y, float z);

    constexpr QVector3D getPosition() const noexcept
    {
        return m_pos;
    }

    void setName(const QString &name)
    {
        m_name->setText(name);
    }

protected:
    void createWidgets();

    QVector3D m_pos;

    QLabel    *m_name = nullptr;
    QLabel *m_x = nullptr;
    QLabel *m_y = nullptr;
    QLabel *m_z = nullptr;

    QHBoxLayout *m_layout = nullptr;
};

};