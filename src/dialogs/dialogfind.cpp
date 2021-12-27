#include "dialogfind.h"
#include "ui_dialogfind.h"


DialogFind::DialogFind(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogFind)
{
    ui->setupUi(this);
    backwardFinding = false;
    caseSensitiveFinding = false;

    connect(ui->buttonCancel, SIGNAL(clicked()),
            this, SLOT(close()));
}

DialogFind::~DialogFind()
{
    delete ui;
}

void DialogFind::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

bool DialogFind::backward() const
{
    return backwardFinding;
}

bool DialogFind::caseSensitive() const
{
    return caseSensitiveFinding;
}

bool DialogFind::findInAllTables() const
{
    return ui->radioAllTables->isChecked();
}

void DialogFind::on_textFind_textChanged(const QString& arg1)
{
    bool textNotEmpty = !arg1.isEmpty();
    ui->buttonNext->setEnabled(textNotEmpty);
    ui->buttonPrevious->setEnabled(textNotEmpty);
}

void DialogFind::on_buttonNext_clicked()
{
    backwardFinding = false;
    emit findNext(ui->textFind->text());
}

void DialogFind::on_buttonPrevious_clicked()
{
    backwardFinding = true;
    emit findPrevious(ui->textFind->text());
}

void DialogFind::on_checkCaseSensitive_stateChanged(int arg1)
{
    caseSensitiveFinding = (arg1 == Qt::CheckState::Checked);
}
