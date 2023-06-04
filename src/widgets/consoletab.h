#pragma once
#include <QWidget>
#include <QBoxLayout>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QLineEdit>

namespace GUI
{

class ConsoleTab : public QWidget
{
    Q_OBJECT
public:
    ConsoleTab(QWidget *parent = nullptr);
    virtual ~ConsoleTab();

    void create();
    
    void appendPlainText(const QString &txt);
    
    void clear();

    int blockCount() const;
    QTextBlock findBlockByNumber(int num);

    void setEnabled(bool enabled = true);

    QString getCommand() const noexcept
    {
        return m_cmdEdit->text();
    }

    void clearCommand()
    {
        m_cmdEdit->clear();
    }

signals:
    void returnPressed();

protected:
    QVBoxLayout *m_layout{nullptr};
    QPlainTextEdit *m_txtConsole{nullptr};
    QLineEdit   *m_cmdEdit{nullptr};
};

}