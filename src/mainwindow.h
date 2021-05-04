#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "OpenTable/onbook.h"


class QStandardItemModel;
class DialogAbout;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void changeEvent(QEvent* event);
    void closeEvent(QCloseEvent* event);

private:
    Ui::MainWindow *ui;
    DialogAbout* windowAbout;
    QList<QStandardItemModel*> modelTables;

    bool modified;
    ONBook book;
    QString lastDirectory;

    void reset();

    bool sureToLeave();
    bool loadBook(const QString& path);
    bool loadDefaultBook();
    bool saveBook(const QString& path = "");

private slots:
    void on_actionFileNew_triggered();
    void on_actionFileOpen_triggered();
    void on_actionFileSave_triggered();
    void on_actionFileSaveAs_triggered();
    void on_actionFileExit_triggered();
    void on_actionHelpAbout_triggered();
};

#endif // MAINWINDOW_H
