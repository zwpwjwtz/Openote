#include <QTranslator>
#include "dialogpreference.h"
#include "ui_dialogpreference.h"
#include "global.h"


DialogPreference::DialogPreference(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::DialogPreference)
{
    ui->setupUi(this);

    languageStringList << "en_US" << "zh_CN";
}

DialogPreference::~DialogPreference()
{
    delete ui;
}

void DialogPreference::changeEvent(QEvent* event)
{
    QDialog::changeEvent(event);
    switch (event->type()) {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void DialogPreference::on_comboLanguage_currentIndexChanged(int index)
{
    appTranslator.load(QString(":/translations/Openote_")
                       .append(languageStringList[index]));
}
