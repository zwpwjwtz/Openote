#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


class DialogAbout;
class DialogPreference;
class BookView;

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
    DialogPreference* windowPreference;
    BookView* bookWidget;
    QString lastDirectory;

    bool sureToLeave();
    bool saveBook(QString path = "");

private slots:
    void on_actionFileNew_triggered();
    void on_actionFileOpen_triggered();
    void on_actionFileSave_triggered();
    void on_actionFileSaveAs_triggered();
    void on_actionFileExit_triggered();
    void on_actionHelpAbout_triggered();
    void on_actionEditPreference_triggered();
};

#endif // MAINWINDOW_H
