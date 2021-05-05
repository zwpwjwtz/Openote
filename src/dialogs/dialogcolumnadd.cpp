#include "dialogcolumnadd.h"
#include "ui_dialogcolumnadd.h"


DialogColumnAdd::DialogColumnAdd(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogColumnAdd)
{
    ui->setupUi(this);
    referring = false;
    typeIndex = 0;
    referenceIndex = 0;
}

DialogColumnAdd::~DialogColumnAdd()
{
    delete ui;
}

void DialogColumnAdd::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}

void DialogColumnAdd::showEvent(QShowEvent* event)
{
    Q_UNUSED(event)
    ui->textName->setText(defaultName);
    ui->radioReferring->setChecked(referring);
    ui->comboReferTable->clear();
    for (int i=0; i<referenceList.length(); i++)
        ui->comboReferTable->addItem(referenceList[i]);
    ui->comboReferTable->setEnabled(referring);
}

void DialogColumnAdd::on_textName_textChanged()
{
    newName = ui->textName->text();
}

void DialogColumnAdd::on_radioEmbeddedType_clicked()
{
    referring = false;
}

void DialogColumnAdd::on_radioReferring_clicked()
{
    referring = true;
}
