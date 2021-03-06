#include <QCloseEvent>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogs/dialogabout.h"
#include "dialogs/dialogpreference.h"
#include "widgets/bookview.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    windowAbout = nullptr;
    windowPreference = nullptr;
    bookWidget = new BookView(this);
    ui->centralWidget->layout()->addWidget(bookWidget);


    connect(ui->actionEditCopy, SIGNAL(triggered()),
            bookWidget, SLOT(copyContent()));
    connect(ui->actionEditCut, SIGNAL(triggered()),
            bookWidget, SLOT(cutContent()));
    connect(ui->actionEditPaste, SIGNAL(triggered()),
            bookWidget, SLOT(pasteContent()));
    connect(ui->actionEditDelete, SIGNAL(triggered()),
            bookWidget, SLOT(deleteContent()));
    connect(ui->actionEditFind, SIGNAL(triggered()),
            bookWidget, SLOT(findContent()));
    connect(ui->actionColumnAdd, SIGNAL(triggered()),
            bookWidget, SLOT(addColumn()));
    connect(ui->actionColumnDelete, SIGNAL(triggered()),
            bookWidget, SLOT(deleteColumn()));
    connect(ui->actionColumnDuplicate, SIGNAL(triggered()),
            bookWidget, SLOT(duplicateColumn()));
    connect(ui->actionColumnRename, SIGNAL(triggered()),
            bookWidget, SLOT(renameColumn()));
    connect(ui->actionColumnToTable, SIGNAL(triggered()),
            bookWidget, SLOT(columnToTable()));
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

    on_actionFileNew_triggered();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent* event)
{
    QMainWindow::changeEvent(event);
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

void MainWindow::setWindowTitle(const QString& newTitle)
{
    QWidget::setWindowTitle(QString(tr("%1 - Openote")).arg(newTitle));
}

bool MainWindow::sureToLeave()
{
    QMessageBox::StandardButton choice =
            QMessageBox::question(this, tr("File not saved"),
                                  tr("The file has been modified but not saved."
                                  "\nDo you want to save it before leave?"),
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
    QString oldPath = bookWidget->currentPath();
    if (path.isEmpty())
    {
        path = oldPath;
        if (path.isEmpty())
        {
            path = QFileDialog::getExistingDirectory(this,
                                                     tr("Save as a directory"),
                                                     lastDirectory);
            if (path.isEmpty())
            {
                QMessageBox::warning(this, tr("File not saved"),
                                     tr("The selected path is not valid!\n"
                                     "Please select a directory for saving."));
                return false;
            }
        }
    }
    if (bookWidget->saveBook(path))
    {
        lastDirectory = path;
        return true;
    }

    QMessageBox::critical(this, tr("File not saved"),
                          tr("An error occured when saving the file."));
    if (oldPath.isEmpty())
        bookWidget->setPath(oldPath);
    return false;
}

void MainWindow::on_menuEdit_triggered()
{
    bool hasSelectedItem = bookWidget->selected();
    ui->actionEditCopy->setEnabled(hasSelectedItem);
    ui->actionEditCut->setEnabled(hasSelectedItem);
    ui->actionEditDelete->setEnabled(hasSelectedItem);
    ui->actionEditPaste->setEnabled(bookWidget->pastingEnabled());
    ui->actionEditFind->setEnabled(bookWidget->count() > 0);
}

void MainWindow::on_actionFileNew_triggered()
{
    if (bookWidget->modified() && !sureToLeave())
        return;

    bookWidget->loadDefaultBook();
    setWindowTitle(tr("Untitled"));
}

void MainWindow::on_actionFileOpen_triggered()
{
    if (bookWidget->modified() && !sureToLeave())
        return;
    QString path = QFileDialog::getExistingDirectory(this, "Open a directory",
                                                     lastDirectory);
    if (path.isEmpty())
        return;

    if (bookWidget->loadBook(path))
    {
        lastDirectory = path;
        QFileInfo file(path);
        setWindowTitle(file.fileName());
    }
    else
        QMessageBox::critical(this, tr("Failed opening file"),
                              QString(tr("An error occurred when opening "
                                         "file %1"))
                                     .arg(path));
}

void MainWindow::on_actionFileSave_triggered()
{
    saveBook();
}

void MainWindow::on_actionFileSaveAs_triggered()
{
    QString oldPath = bookWidget->currentPath();
    QString path = QFileDialog::getExistingDirectory(this,
                                                     tr("Save as a directory"),
                                                     lastDirectory);
    if (path.isEmpty())
        return;

    if (saveBook(path))
    {
        lastDirectory = path;
        return;
    }

    QMessageBox::critical(this, tr("File not saved"),
                          tr("An error occured when saving the file."));
    if (oldPath.isEmpty())
        bookWidget->setPath(oldPath);
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

void MainWindow::on_actionEditPreference_triggered()
{
    if (!windowPreference)
        windowPreference = new DialogPreference(this);
    windowPreference->show();
}
