#include <QCloseEvent>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogabout.h"
#include "widgets/bookview.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    windowAbout = nullptr;
    bookWidget = new BookView(this);
    ui->centralWidget->layout()->addWidget(bookWidget);

    move((QApplication::desktop()->width() - width()) / 2,
         (QApplication::desktop()->height() - height()) / 2);

    bookWidget->loadDefaultBook();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (bookWidget->modified())
    {
        if (!sureToLeave())
            event->ignore();
    }
}

bool MainWindow::sureToLeave()
{
    QMessageBox::StandardButton choice =
            QMessageBox::question(this, "File not saved",
                                  "File has been modified but not saved.\n"
                                  "Do you want to save it before leave?",
                                  QMessageBox::Yes | QMessageBox::No |
                                  QMessageBox::Cancel);

    if (choice == QMessageBox::Yes)
        return bookWidget->saveBook();
    else if (choice == QMessageBox::No)
        return true;
    else
        return false;
}

void MainWindow::on_actionFileNew_triggered()
{
    if (bookWidget->modified() && !sureToLeave())
        return;

    bookWidget->loadDefaultBook();
}

void MainWindow::on_actionFileOpen_triggered()
{
    if (bookWidget->modified() && !sureToLeave())
        return;
    QString path = QFileDialog::getExistingDirectory(this, "Open a directory",
                                                     lastDirectory);
    if (!path.isEmpty())
        bookWidget->loadBook(path);
}

void MainWindow::on_actionFileSave_triggered()
{
    bookWidget->saveBook();
}

void MainWindow::on_actionFileSaveAs_triggered()
{
    QString path = QFileDialog::getExistingDirectory(this, "Save as a directory",
                                                     lastDirectory);
    if (!path.isEmpty())
        bookWidget->saveBook(path);
}

void MainWindow::on_actionFileExit_triggered()
{
    close();
}

void MainWindow::on_actionHelpAbout_triggered()
{
    if (!windowAbout)
        windowAbout = new DialogAbout(this);
    windowAbout->show();
}
