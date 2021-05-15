#ifndef DIALOGPREFERENCE_H
#define DIALOGPREFERENCE_H

#include <QDialog>

namespace Ui {
class DialogPreference;
}

class DialogPreference : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPreference(QWidget* parent = nullptr);
    ~DialogPreference();

protected:
    void changeEvent(QEvent* event);

private:
    Ui::DialogPreference *ui;
    QList<QString> languageStringList;

private slots:
    void on_comboLanguage_currentIndexChanged(int index);
};

#endif // DIALOGPREFERENCE_H
