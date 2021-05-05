#include <QStandardItemModel>
#include <QTableView>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include "bookview.h"
#include <dialogs/dialogcolumnadd.h>

#define OPENOTE_BOOKVIEW_PROP_TABLE_ID    "table-id"
#define OPENOTE_BOOKVIEW_ROLE_COLUMN_ID   Qt::UserRole + 2
#define OPENOTE_BOOKVIEW_ROLE_COLUMN_TYPE Qt::UserRole + 3


BookView::BookView(QWidget *parent) : QTabWidget(parent)
{
    dialogColumnAdd = nullptr;

    setDocumentMode(true);
    setTabPosition(TabPosition::South);

    clear();
}

void BookView::clear()
{
    QTabWidget::clear();
    modelTables.clear();
    book.clear();
    isModified = false;
}

bool BookView::loadBook(const QString& path)
{
    clear();

    book.setBindingDirectory(path.toStdString());
    if (!book.load())
        return false;

    ONTable* table;
    std::vector<int> tableIDs = book.tableIDs();
    std::vector<int> columnIDs;
    std::list<int> rowIDs;
    QList<int> tableIDList, columnIDList, rowIDList;
    QStringList columnNameList;
    ONTable::ColumnType columnType;
    QStandardItemModel* model;
    QStandardItem* newItem;
    QList<QStandardItem*> itemList;
    for (auto i=tableIDs.cbegin(); i!=tableIDs.cend(); i++)
    {
        if (tableIDList.indexOf(*i) >= 0)
        {
            // Table with duplicated ID; skip it
            continue;
        }

        // Load data of each book table into a model
        table = book.table(*i);
        rowIDs = table->IDs();
        columnIDs = table->columnIDs();
        columnIDList.clear();
        columnNameList.clear();
        model = new QStandardItemModel(this);
        model->setColumnCount(0);
        for (auto j=columnIDs.cbegin(); j!=columnIDs.cend(); j++)
        {
            if (columnIDList.indexOf(*j) >= 0)
            {
                // Column with duplicated ID; skip it
                continue;
            }

            columnNameList.push_back(
                            QString::fromStdString(table->columnName(*j)));
            columnType = table->columnType(*j);
            rowIDList.clear();
            itemList.clear();
            for (auto k=rowIDs.cbegin(); k!=rowIDs.cend(); k++)
            {
                if (rowIDList.indexOf(*k) >= 0)
                {
                    // Row with duplicated ID; skip it
                    continue;
                }

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
                if (newItem == nullptr)
                    continue;
                itemList.push_back(newItem);
                rowIDList.push_back(*k);
            }
            model->appendColumn(itemList);
            model->setHeaderData(*j, Qt::Horizontal, *j,
                                 OPENOTE_BOOKVIEW_ROLE_COLUMN_ID);
            model->setHeaderData(*j, Qt::Horizontal, int(columnType),
                                 OPENOTE_BOOKVIEW_ROLE_COLUMN_TYPE);
            columnIDList.push_back(*j);
        }
        model->setHorizontalHeaderLabels(columnNameList);
        modelTables.push_back(model);
        tableIDList.push_back(*i);

        // Bind the model with a view
        QTableView* viewTable = new QTableView(this);
        viewTable->setModel(model);
        viewTable->setProperty(OPENOTE_BOOKVIEW_PROP_TABLE_ID, *i);
        addTab(viewTable, QString::fromStdString(book.tableName(*i)));
    }

    return true;
}

bool BookView::loadDefaultBook()
{
    clear();

    // Add an empty table
    QString newTableName("Table1");
    int newTableID = book.addTable(newTableName.toStdString());
    QStandardItemModel* model = new QStandardItemModel(this);
    model->setColumnCount(1);
    model->setRowCount(1);
    model->setHorizontalHeaderItem(0, new QStandardItem("New Column"));
    modelTables.push_back(model);
    QTableView* viewTable = new QTableView(this);
    viewTable->setModel(model);
    viewTable->setProperty(OPENOTE_BOOKVIEW_PROP_TABLE_ID, newTableID);
    addTab(viewTable, newTableName);
    return true;
}

bool BookView::saveBook(const QString& path)
{
    book.setBindingDirectory(path.toStdString());
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
    return QString::fromStdString(book.bindingDirectory());
}

bool BookView::modified() const
{
    return isModified;
}

bool BookView::addColumn()
{
    BookIndex index = getCurrentIndex();
    if (index.table < 0)
        return false;

    if (dialogColumnAdd == nullptr)
        dialogColumnAdd = new DialogColumnAdd(this);
    dialogColumnAdd->exec();
    if (dialogColumnAdd->result() == QDialog::Rejected ||
        dialogColumnAdd->newName.isEmpty())
        return false;
    if (dialogColumnAdd->referring)
    {

    }
    else
    {
        // TODO: set column type for the new column
        QStandardItemModel* table = modelTables[index.table];
        table->setColumnCount(table->columnCount() + 1);
        table->setHeaderData(table->columnCount() - 1, Qt::Horizontal,
                             dialogColumnAdd->newName);
    }

    isModified = true;
    return true;
}

bool BookView::deleteColumn()
{
    BookIndex index = getCurrentIndex();
    if (!index.isValid())
        return false;
    if (modelTables[index.table]->removeColumn(index.column))
    {
        isModified = true;
        return true;
    }
    else
        return false;
}

bool BookView::duplicateColumn()
{
    BookIndex index = getCurrentIndex();
    if (index.table < 0 || index.column < 0)
        return false;

    QString oldName = columnHeader(index.table, index.column);
    QString newName = QInputDialog::getText(this, "Duplicate a column",
                                            "New column name:",
                                            QLineEdit::Normal,
                                            oldName);
    if (newName.isEmpty())
        return false;

    QStandardItemModel* table = modelTables[index.table];
    QList<QStandardItem*> columnItems;
    QStandardItem* oldItem, *newItem;
    for (int i=0; i<table->rowCount(); i++)
    {
        oldItem = table->item(i, index.column);
        if (oldItem == nullptr)
            newItem = new QStandardItem();
        else
            newItem = new QStandardItem(oldItem->text());
        columnItems.push_back(newItem);
    }
    table->appendColumn(columnItems);
    setColumnHeader(newName, index.table, table->columnCount() - 1);

    isModified = true;
    return true;
}

bool BookView::renameColumn()
{
    BookIndex index = getCurrentIndex();
    if (!index.isValid())
        return false;

    QString oldName = columnHeader(index.table, index.column);
    QString newName = QInputDialog::getText(this, "Rename a column",
                                            "New name for the column:",
                                            QLineEdit::Normal,
                                            oldName);
    if (newName == oldName)
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
    BookIndex index = getCurrentIndex();
    if (index.table < 0)
        return false;
    QStandardItemModel* table = modelTables[index.table];
    table->setRowCount(table->rowCount() + 1);

    isModified = true;
    return true;
}

bool BookView::deleteRow()
{
    BookIndex index = getCurrentIndex();
    if (!index.isValid())
        return false;
    modelTables[index.table]->removeRow(index.row);

    isModified = true;
    return true;
}

bool BookView::duplicateRow()
{
    BookIndex index = getCurrentIndex();
    if (!index.isValid())
        return false;
    QStandardItemModel* table = modelTables[index.table];
    QList<QStandardItem*> rowItems;
    QStandardItem* oldItem, *newItem;
    for (int i=0; i<table->columnCount(); i++)
    {
        oldItem = table->item(index.row, i);
        if (oldItem == nullptr)
            newItem = new QStandardItem();
        else
            newItem = new QStandardItem(oldItem->text());
        rowItems.push_back(newItem);
    }
    table->appendRow(rowItems);

    isModified = true;
    return true;
}

bool BookView::addTable()
{
    QString newName = QInputDialog::getText(this, "Add a table",
                                            "Table name:",
                                            QLineEdit::Normal,
                                            "NewTable");
    if (newName.isEmpty())
        return false;

    int newTableID = book.addTable(newName.toStdString());
    QStandardItemModel* newModel = new QStandardItemModel(this);
    modelTables.push_back(newModel);
    QTableView* viewTable = new QTableView(this);
    viewTable->setModel(newModel);
    viewTable->setProperty(OPENOTE_BOOKVIEW_PROP_TABLE_ID, newTableID);
    addTab(viewTable, newName);

    isModified = true;
    return true;
}

bool BookView::deleteTable()
{
    // TODO: check for reference before deleting
    BookIndex index = getCurrentIndex();
    if (index.table < 0)
        return false;
    if (QMessageBox::warning(this, "Delete a table",
                             QString("Are you sure to delete table %1 ?")
                                    .arg(QString::fromStdString(
                                             book.tableName(index.table))))
            != QMessageBox::Yes)
        return false;

    removeTab(currentIndex());
    modelTables.removeAt(index.table);
    book.removeTable(getTableID(index.table));

    isModified = true;
    return true;
}

bool BookView::duplicateTable()
{
    BookIndex index = getCurrentIndex();
    if (index.table < 0)
        return false;

    QString oldName = QString::fromStdString(
                                    book.tableName(getTableID(index.table)));
    QString newName = QInputDialog::getText(this, "Duplicate a table",
                                            "New table name:",
                                            QLineEdit::Normal,
                                            QString(oldName).append("Copy"));
    if (newName.isEmpty())
        return false;

    // TODO: duplicate the table content
    int newTableID = book.addTable(newName.toStdString());
    QStandardItemModel* newModel =
                             new QStandardItemModel(modelTables[index.table]);
    modelTables.push_back(newModel);
    QTableView* viewTable = new QTableView(this);
    viewTable->setModel(newModel);
    viewTable->setProperty(OPENOTE_BOOKVIEW_PROP_TABLE_ID, newTableID);
    addTab(viewTable, newName);

    isModified = true;
    return true;
}

bool BookView::renameTable()
{
    BookIndex index = getCurrentIndex();
    if (index.table < 0)
        return false;

    QString oldName = QString::fromStdString(book.tableName(index.table));
    QString newName = QInputDialog::getText(this, "Rename a table",
                                            "New name for the column:",
                                            QLineEdit::Normal,
                                            oldName);
    if (newName.isEmpty())
        return false;
    if (newName == oldName)
        return true;
    if (book.setTableName(index.table, newName.toStdString()))
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

int BookView::getColumnID(int tableIndex, int columnIndex) const
{
    return modelTables[tableIndex]->headerData(columnIndex,
                                               Qt::Horizontal,
                                               OPENOTE_BOOKVIEW_ROLE_COLUMN_ID)
                                 .toInt();
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
    return modelTables[tableIndex]->headerData(columnIndex,
                                               Qt::Horizontal).toString();
}

bool BookView::setColumnHeader(const QString &text,
                               int tableIndex, int columnIndex)
{
    modelTables[tableIndex]->setHeaderData(columnIndex, Qt::Horizontal, text);
    return true;
}
