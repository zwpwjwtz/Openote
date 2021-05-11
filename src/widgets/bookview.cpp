#include <QTableView>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include "bookview.h"
#include "columnreferencedelegate.h"
#include "models/tablemodel.h"
#include <dialogs/dialogcolumnadd.h>

#define OPENOTE_BOOKVIEW_PROP_TABLE_ID    "table-id"


BookView::BookView(QWidget *parent) : QTabWidget(parent)
{
    dialogColumnAdd = nullptr;
    referenceDelegate = new ColumnReferenceDelegate();
    referenceDelegate->book = &book;

    setDocumentMode(true);
    setTabPosition(TabPosition::South);

    clear();
}

void BookView::clear()
{
    QTabWidget::clear();
    book.clear();
    book.setPath("");
    isModified = false;
}

bool BookView::loadBook(const QString& path)
{
    clear();

    book.setPath(path);
    if (!book.load())
        return false;

    auto tableIDs = book.tableIDs();
    for (auto i=tableIDs.cbegin(); i!=tableIDs.cend(); i++)
    {
        // Bind the model with a view
        QTableView* viewTable = new QTableView(this);
        viewTable->setModel(book.table(*i));
        viewTable->setItemDelegate(referenceDelegate);
        viewTable->setProperty(OPENOTE_BOOKVIEW_PROP_TABLE_ID, *i);
        connect(book.table(*i),
                SIGNAL(dataChanged(const QModelIndex, const QModelIndex,
                                   const QVector<int>)),
                this, SLOT(onTableDataChanged()));
        addTab(viewTable, book.tableName(*i));
    }

    return true;
}

bool BookView::loadDefaultBook()
{
    clear();

    // Add an empty table
    QString newTableName("Table1");
    TableModel* newTable = book.addTable(newTableName);
    if (newTable == nullptr)
        return false;

    newTable->newColumn("New Column", TableModel::ColumnType::String);
    newTable->newRow();
    QTableView* viewTable = new QTableView(this);
    viewTable->setModel(newTable);
    viewTable->setItemDelegate(referenceDelegate);
    viewTable->setProperty(OPENOTE_BOOKVIEW_PROP_TABLE_ID, newTable->ID);
    connect(newTable, SIGNAL(dataChanged(const QModelIndex, const QModelIndex,
                                         const QVector<int>)),
            this, SLOT(onTableDataChanged()));
    addTab(viewTable, newTableName);
    return true;
}

bool BookView::saveBook(const QString& path)
{
    book.setPath(path);
    if (book.save())
    {
        isModified = false;
        return true;
    }
    else
        return false;
}

QString BookView::currentPath() const
{
    return book.path();
}

void BookView::setPath(const QString &newPath)
{
    book.setPath(newPath);
}

bool BookView::modified() const
{
    return isModified;
}

bool BookView::addColumn()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (index.table < 0)
        return false;

    if (dialogColumnAdd == nullptr)
    {
        dialogColumnAdd = new DialogColumnAdd(this);
        connect(dialogColumnAdd, SIGNAL(finished(int)),
                this, SLOT(onDialogColumnAddFinished(int)));
    }
    if (book.table(getTableID(index.table))->countColumn() < 1)
    {
        dialogColumnAdd->enableReference = false;
        dialogColumnAdd->referring = false;
    }
    else
    {
        dialogColumnAdd->enableReference = true;
        dialogColumnAdd->referenceList.clear();
        auto tableIDs = book.tableIDs();
        for (auto i=tableIDs.cbegin(); i!=tableIDs.cend(); i++)
            dialogColumnAdd->referenceList.push_back(book.tableName(*i));
    }

    dialogColumnAdd->open();
    return true;
}

bool BookView::deleteColumn()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (!index.isValid())
        return false;

    book.table(getTableID(index.table))->removeColumns(index.column,1 );
    isModified = true;
        return true;
}

bool BookView::duplicateColumn()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (index.table < 0 || index.column < 0)
        return false;

    QString oldName = columnHeader(index.table, index.column);
    QString newName = QInputDialog::getText(this, "Duplicate a column",
                                            "New column name:",
                                            QLineEdit::Normal,
                                            oldName);
    if (newName.isEmpty())
        return false;

    TableModel* table = book.table(getTableID(index.table));
    if (table->duplicateColumn(index.column, newName))
    {

        isModified = true;
        return true;
    }
    else
        return false;
}

bool BookView::renameColumn()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (!index.isValid())
        return false;

    QString oldName = columnHeader(index.table, index.column);
    QString newName = QInputDialog::getText(this, "Rename a column",
                                            "New name for the column:",
                                            QLineEdit::Normal,
                                            oldName);
    newName.replace("\n", "");
    newName.replace("\"", "'");
    if (newName.isEmpty() || newName == oldName)
        return true;

    if (setColumnHeader(newName, index.table, index.column))
    {
        isModified = true;
        return true;
    }
    else
        return false;
}

bool BookView::addRow()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (index.table < 0)
        return false;

    TableModel* table = book.table(getTableID(index.table));
    if (table->countColumn() == 0)
    {
        QMessageBox::warning(this, "No column presents",
                             "Please add a column before adding rows.");
        return false;
    }
    table->newRow();

    isModified = true;
    return true;
}

bool BookView::deleteRow()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (!index.isValid())
        return false;

    TableModel* table = book.table(getTableID(index.table));
    table->removeRows(index.row, 1);

    isModified = true;
    return true;
}

bool BookView::duplicateRow()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (!index.isValid())
        return false;

    TableModel* table = book.table(getTableID(index.table));
    if (table->duplicateRow(index.row))
    {
        isModified = true;
        return true;
    }
    else
        return false;
}

bool BookView::addTable()
{
    QString newName = QInputDialog::getText(this, "Add a table",
                                            "Table name:",
                                            QLineEdit::Normal,
                                            "NewTable");
    if (newName.isEmpty())
        return false;

    TableModel* newTable = book.addTable(newName);
    QTableView* viewTable = new QTableView(this);
    viewTable->setModel(newTable);
    viewTable->setItemDelegate(referenceDelegate);
    viewTable->setProperty(OPENOTE_BOOKVIEW_PROP_TABLE_ID, newTable->ID);
    connect(newTable, SIGNAL(dataChanged(const QModelIndex, const QModelIndex,
                                         const QVector<int>)),
            this, SLOT(onTableDataChanged()));
    addTab(viewTable, newName);
    setCurrentWidget(viewTable);

    isModified = true;
    return true;
}

bool BookView::deleteTable()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (index.table < 0)
        return false;

    int tableID = getTableID(index.table);
    if (QMessageBox::warning(this, "Delete a table",
                             QString("Are you sure to delete table %1 ?")
                                    .arg(book.tableName(tableID)),
                             QMessageBox::Yes | QMessageBox::No)
            != QMessageBox::Yes)
        return false;

    if (book.removeTable(getTableID(index.table)))
    {
        removeTab(currentIndex());
        isModified = true;
        return true;
    }
    else
        return false;
}

bool BookView::duplicateTable()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (index.table < 0)
        return false;

    int oldTableID = getTableID(index.table);
    QString oldName = book.tableName(oldTableID);
    QString newName = QInputDialog::getText(this, "Duplicate a table",
                                            "New table name:",
                                            QLineEdit::Normal,
                                            QString(oldName).append("Copy"));
    if (newName.isEmpty())
        return false;

    // TODO: duplicate the table content
    TableModel* newTable = book.duplicateTable(oldTableID, newName);
    if (newTable == nullptr)
        return false;

    QTableView* viewTable = new QTableView(this);
    viewTable->setModel(newTable);
    viewTable->setItemDelegate(referenceDelegate);
    viewTable->setProperty(OPENOTE_BOOKVIEW_PROP_TABLE_ID, newTable->ID);
    connect(newTable, SIGNAL(dataChanged(const QModelIndex, const QModelIndex,
                                         const QVector<int>)),
            this, SLOT(onTableDataChanged()));
    addTab(viewTable, newName);

    isModified = true;
    return true;
}

bool BookView::renameTable()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (index.table < 0)
        return false;

    int tableID = getTableID(index.table);
    QString oldName = book.tableName(tableID);
    QString newName = QInputDialog::getText(this, "Rename a table",
                                            "New name for the column:",
                                            QLineEdit::Normal,
                                            oldName);
    newName.replace("\n", "");
    newName.replace("\"", "'");
    if (newName.isEmpty() || newName == oldName)
        return true;

    if (book.setTableName(tableID, newName))
    {
        setTabText(currentIndex(), newName);
        isModified = true;
        return true;
    }
    return false;
}

int BookView::getTableID(int tableIndex) const
{
    return widget(tableIndex)->property(OPENOTE_BOOKVIEW_PROP_TABLE_ID).toInt();
}

int BookView::getTableIndex(int tableID) const
{
    auto tableIDList = book.tableIDs();
    auto index = std::find(tableIDList.cbegin(), tableIDList.cend(), tableID);
    if (index == tableIDList.cend())
        return -1;
    else
        return int(index - tableIDList.cbegin());
}

BookView::BookIndex BookView::getCurrentIndex() const
{
    BookIndex bookIndex{-1, -1, -1};

    if (!currentWidget())
        return bookIndex;
    bookIndex.table = getTableIndex(getTableID(currentIndex()));

    // TODO: deal with tables of empty row but of non-empty columns
    QModelIndex modelIndex =
                    dynamic_cast<QTableView*>(currentWidget())->currentIndex();
    if (!modelIndex.isValid())
        return bookIndex;

    bookIndex.column = modelIndex.column();
    bookIndex.row = modelIndex.row();
    return bookIndex;
}

QString BookView::columnHeader(int tableIndex, int columnIndex) const
{
    return book.table(getTableID(tableIndex))->
                            headerData(columnIndex, Qt::Horizontal).toString();
}

bool BookView::setColumnHeader(const QString &text,
                               int tableIndex, int columnIndex)
{
    book.table(getTableID(tableIndex))->
                            setHeaderData(columnIndex, Qt::Horizontal, text);
    return true;
}

void BookView::onDialogColumnAddFinished(int result)
{
    if (result == QDialog::Rejected || dialogColumnAdd->newName.isEmpty())
        return;

    int tableID = getTableID(currentBookIndex.table);
    int referenceTableID;
    if (dialogColumnAdd->referring)
    {
        auto tableIDs = book.tableIDs();
        referenceTableID = tableIDs[dialogColumnAdd->referenceIndex];
        if (referenceTableID == tableID)
        {
            QMessageBox::warning(this, "Failed creating column",
                                 "Creating a column referring to its "
                                 "parent table is not allowed. \n"
                                 "Please select another reference table.");
            dialogColumnAdd->open();
            return;
        }
    }

    TableModel* table = book.table(tableID);
    std::string newName = dialogColumnAdd->newName.toStdString();
    if (dialogColumnAdd->referring)
    {
        if (table->newColumn(newName,
                             TableModel::ColumnType::IntegerList,
                             referenceTableID) <= 0)
        {
            QMessageBox::critical(this, "Failed creating column",
                                  "An error occurred when creating a column "
                                  "referring to another table.");
            return;
        }
    }
    else
    {
        switch (dialogColumnAdd->typeIndex)
        {
            case 0:  // Integer
                table->newColumn(newName, TableModel::ColumnType::Integer);
                break;
            case 1:  // Double
                table->newColumn(newName, TableModel::ColumnType::Double);
                break;
            case 2:  // String
                table->newColumn(newName, TableModel::ColumnType::String);
                break;
            default:;
        }
    }

    isModified = true;
}

void BookView::onTableDataChanged()
{
    isModified = true;
}
