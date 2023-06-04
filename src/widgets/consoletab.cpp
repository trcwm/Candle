#include "consoletab.h"
#include <QDebug>

namespace GUI
{

ConsoleTab::ConsoleTab(QWidget *parent) : QWidget(parent)
{
    create();
}

ConsoleTab::~ConsoleTab()
{
}

void ConsoleTab::create()
{
    m_layout = new QVBoxLayout();

    m_txtConsole = new QPlainTextEdit();
    m_txtConsole->setReadOnly(true);
    m_txtConsole->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_cmdEdit = new QLineEdit();
    m_cmdEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    m_layout->addWidget(m_txtConsole,1);
    m_layout->addWidget(m_cmdEdit);
    
    setLayout(m_layout);

    connect(m_cmdEdit, &QLineEdit::returnPressed, this, &ConsoleTab::returnPressed);
}

void ConsoleTab::appendPlainText(const QString &txt)
{
    m_txtConsole->appendPlainText(txt);
}

int ConsoleTab::blockCount() const 
{
    return m_txtConsole->blockCount();
}

QTextBlock ConsoleTab::findBlockByNumber(int num)
{
    return m_txtConsole->document()->findBlockByNumber(num);
}

void ConsoleTab::clear()
{
    m_txtConsole->clear();
}

void ConsoleTab::setEnabled(bool enabled)
{
    m_cmdEdit->setEnabled(enabled);
}

};
