#include <QCloseEvent>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogs/dialogabout.h"
#include "widgets/bookview.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    windowAbout = nullptr;
    bookWidget = new BookView(this);
    ui->centralWidget->layout()->addWidget(bookWidget);

    connect(ui->actionColumnAdd, SIGNAL(triggered()),
            bookWidget, SLOT(addColumn()));
    connect(ui->actionColumnDelete, SIGNAL(triggered()),
            bookWidget, SLOT(deleteColumn()));
    connect(ui->actionColumnDuplicate, SIGNAL(triggered()),
            bookWidget, SLOT(duplicateColumn()));
    connect(ui->actionColumnRename, SIGNAL(triggered()),
            bookWidget, SLOT(renameColumn()));
    connect(ui->actionRowAdd, SIGNAL(triggered()),
            bookWidget, SLOT(addRow()));
    connect(ui->actionRowDelete, SIGNAL(triggered()),
            bookWidget, SLOT(deleteRow()));
    connect(ui->actionRowDuplicate, SIGNAL(triggered()),
            bookWidget, SLOT(duplicateRow()));
    connect(ui->actionTableAdd, SIGNAL(triggered()),
            bookWidget, SLOT(addTable()));
    connect(ui->actionTableDelete, SIGNAL(triggered()),
            bookWidget, SLOT(deleteTable()));
    connect(ui->actionTableDuplicate, SIGNAL(triggered()),
            bookWidget, SLOT(duplicateTable()));
    connect(ui->actionTableRename, SIGNAL(triggered()),
            bookWidget, SLOT(renameTable()));

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
        return saveBook();
    else if (choice == QMessageBox::No)
        return true;
    else
        return false;
}

bool MainWindow::saveBook(QString path)
{
    if (path.isEmpty())
    {
        path = bookWidget->currentPath();
        if (path.isEmpty())
        {
            path = QFileDialog::getExistingDirectory(this,
                                                     "Save as a directory",
                                                     lastDirectory);
            if (path.isEmpty())
            {
                QMessageBox::warning(this, "Book not saved",
                                     "The selected path is not valid!\n"
                                     "Please select a directory for saving.");
                return false;
            }
        }
    }
    return bookWidget->saveBook(path);
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
    saveBook();
}

void MainWindow::on_actionFileSaveAs_triggered()
{
    QString path = QFileDialog::getExistingDirectory(this, "Save as a directory",
                                                     lastDirectory);
    if (!path.isEmpty())
        saveBook(path);
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
