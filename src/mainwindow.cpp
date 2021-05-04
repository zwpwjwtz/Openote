#include <QCloseEvent>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QTableView>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogabout.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    modified = false;
    windowAbout = nullptr;

    move((QApplication::desktop()->width() - width()) / 2,
         (QApplication::desktop()->height() - height()) / 2);

    reset();
    loadDefaultBook();
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
    if (modified)
    {
        if (!sureToLeave())
            event->ignore();
    }
}

void MainWindow::reset()
{
    // Clear existing tables
    ui->tabWidget->clear();
    modelTables.clear();

    modified = false;
}

bool MainWindow::loadBook(const QString& path)
{
    book.setBindingDirectory(path.toStdString());
    if (!book.load())
        return false;

    reset();

    ONTable* table;
    std::vector<int> tableIDs = book.tableIDs();
    std::vector<int> columnIDs;
    std::list<int> rowIDs;
    ONTable::ColumnType columnType;
    QStandardItemModel* model;
    QStandardItem* newItem;
    QList<QStandardItem*> itemList;
    for (auto i=tableIDs.cbegin(); i!=tableIDs.cend(); i++)
    {
        model = new QStandardItemModel(this);

        // Load data of each book table into a model
        table = book.table(*i);
        columnIDs = table->columnIDs();
        rowIDs = table->IDs();
        for (auto j=columnIDs.cbegin(); j!=columnIDs.cend(); j++)
        {
            columnType = table->columnType(*j);
            itemList.clear();
            for (auto k=rowIDs.cbegin(); k!=rowIDs.cend(); k++)
            {
                switch (columnType)
                {
                    case ONTable::ColumnType::Integer:
                        newItem = new QStandardItem(
                                    QString::number(table->readInt(*k, *j)));
                        break;
                    case ONTable::ColumnType::Double:
                        newItem = new QStandardItem(
                                    QString::number(table->readDouble(*k, *j)));
                        break;
                    case ONTable::ColumnType::String:
                        newItem = new QStandardItem(QString::fromStdString(
                                                    table->readString(*k, *j)));
                        break;
                    default:
                        newItem = nullptr;
                }
                if (newItem != nullptr)
                    itemList.push_back(newItem);
            }
            model->appendColumn(itemList);
        }
        modelTables.push_back(model);

        // Bind the model with a view
        QTableView* viewTable = new QTableView(this);
        viewTable->setModel(model);
        ui->tabWidget->addTab(viewTable,
                              QString::fromStdString(book.tableName(*i)));
    }
}

bool MainWindow::loadDefaultBook()
{
    // Add an empty table
    QStandardItemModel* model = new QStandardItemModel(this);
    modelTables.push_back(model);
    QTableView* viewTable = new QTableView(this);
    viewTable->setModel(model);
    ui->tabWidget->addTab(viewTable, "Table1");
    return true;
}

bool MainWindow::saveBook(const QString& path)
{
    book.setBindingDirectory(path.toStdString());
    if (book.save())
    {
        modified = false;
        return true;
    }
    else
        return false;
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

void MainWindow::on_actionFileNew_triggered()
{
    if (modified && !sureToLeave())
        return;

    reset();
    loadDefaultBook();
}

void MainWindow::on_actionFileOpen_triggered()
{
    if (modified && !sureToLeave())
        return;
    QString path = QFileDialog::getExistingDirectory(this, "Open a directory",
                                                     lastDirectory);
    if (!path.isEmpty())
        loadBook(path);
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
