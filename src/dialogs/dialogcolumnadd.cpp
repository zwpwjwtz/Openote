#include "dialogcolumnadd.h"
#include "ui_dialogcolumnadd.h"


DialogColumnAdd::DialogColumnAdd(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogColumnAdd)
{
    ui->setupUi(this);
    enableReference = true;
    referring = false;
    typeIndex = 0;
    referenceIndex = 0;
    insertPosition = -1;
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
    ui->radioEmbeddedType->setChecked(!referring);
    ui->radioReferring->setEnabled(enableReference);
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
    ui->comboEmbeddedType->setEnabled(true);
    ui->comboReferTable->setEnabled(false);
}

void DialogColumnAdd::on_radioReferring_clicked()
{
    referring = true;
    ui->comboEmbeddedType->setEnabled(false);
    ui->comboReferTable->setEnabled(true);
}

void DialogColumnAdd::on_comboEmbeddedType_currentIndexChanged(int index)
{
    typeIndex = index;
}

void DialogColumnAdd::on_comboReferTable_currentIndexChanged(int index)
{
    referenceIndex = index;
}
