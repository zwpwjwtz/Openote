#include "dialogabout.h"
#include "ui_dialogabout.h"

DialogAbout::DialogAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAbout)
{
    ui->setupUi(this);
}

DialogAbout::~DialogAbout()
{
    delete ui;
}

void DialogAbout::changeEvent(QEvent* event)
{
    QDialog::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);

    ui->labelVersion->setText(QString(tr("Ver: %1")).arg(APP_VERSION));
    ui->textAbout->setPlainText(tr("Openote - Smart tool for managing operation notes\n\n"
                    "This program is a free software.\n\n"
                    "You can redistribute it and/or modify it under the terms of "
                    "the GNU Library General Public License as published by "
                    "the Free Software Foundation; either version 3 of the License, "                  "or (at your option) any later version.\n\n"
                    "This program is distributed in the hope that it will be useful, but WITHOUT "
                    "ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or "
                    "FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License "
                    "for more details.\n"));
}
