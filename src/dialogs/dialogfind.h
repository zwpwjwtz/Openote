#ifndef DIALOGFIND_H
#define DIALOGFIND_H

#include <QDialog>

namespace Ui {
class DialogFind;
}

class DialogFind : public QDialog
{
    Q_OBJECT

public:
    explicit DialogFind(QWidget *parent = nullptr);
    ~DialogFind();

    bool backward() const;
    bool caseSensitive() const;
    bool findInAllTables() const;

signals:
    void findNext(QString text);
    void findPrevious(QString text);

protected:
    void changeEvent(QEvent *e);

private slots:
    void on_textFind_textChanged(const QString &arg1);
    void on_buttonNext_clicked();
    void on_buttonPrevious_clicked();

    void on_checkCaseSensitive_stateChanged(int arg1);

private:
    Ui::DialogFind *ui;
    bool backwardFinding;
    bool caseSensitiveFinding;
};

#endif // DIALOGFIND_H
