// This file is a part of "CandleM" application.
// Copyright 2015-2016 Hayrullin Denis Ravilevich
// Copyright 2023 Niels Moseley

#include <QDesktopServices>
#include "frmabout.h"
#include "ui_frmabout.h"

frmAbout::frmAbout(QWidget *parent) : QDialog(parent),
                                      ui(new Ui::frmAbout)
{
    ui->setupUi(this);

    ui->lblAbout->setText(ui->lblAbout->text().arg(qApp->applicationVersion()));
    QFile file(":/LICENSE");

    if (file.open(QIODevice::ReadOnly))
    {
        ui->txtLicense->setPlainText(file.readAll());
    }

    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui->txtLicense->setFont(fixedFont);
}

frmAbout::~frmAbout()
{
    delete ui;
}

void frmAbout::on_cmdOk_clicked()
{
    this->hide();
}

void frmAbout::on_lblAbout_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}
