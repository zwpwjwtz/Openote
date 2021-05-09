#ifndef DIALOGCOLUMNADD_H
#define DIALOGCOLUMNADD_H

#include <QDialog>


namespace Ui {
class DialogColumnAdd;
}

class DialogColumnAdd : public QDialog
{
    Q_OBJECT

public:
    QString defaultName;
    QString newName;
    bool enableReference;
    bool referring;
    int typeIndex;
    int referenceIndex;
    QList<QString> referenceList;

    explicit DialogColumnAdd(QWidget *parent = nullptr);
    ~DialogColumnAdd();

protected:
    void changeEvent(QEvent* event);
    void showEvent(QShowEvent* event);

private:
    Ui::DialogColumnAdd *ui;

private slots:
    void on_textName_textChanged();
    void on_radioEmbeddedType_clicked();
    void on_radioReferring_clicked();
    void on_comboEmbeddedType_currentIndexChanged(int index);
    void on_comboReferTable_currentIndexChanged(int index);
};

#endif // DIALOGCOLUMNADD_H
