#include <QStandardItemModel>
#include <QTableView>
#include <QHeaderView>
#include "bookview.h"

#define OPENOTE_BOOKVIEW_PROP_TABLE_ID    "table-id"
#define OPENOTE_BOOKVIEW_PROP_COLUMN_ID   "column-id"


BookView::BookView(QWidget *parent) : QTabWidget(parent)
{
    setDocumentMode(true);
    setTabPosition(TabPosition::South);

    clear();
}

void BookView::clear()
{
    QTabWidget::clear();
    modelTables.clear();
    isModified = false;
}

bool BookView::loadBook(const QString& path)
{
    book.setBindingDirectory(path.toStdString());
    if (!book.load())
        return false;

    if (!modified())
        clear();

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
            model->setHeaderData(*j, Qt::Horizontal, *j, Qt::UserRole);
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
    // Add an empty table
    clear();
    QStandardItemModel* model = new QStandardItemModel(this);
    model->setColumnCount(1);
    model->setRowCount(1);
    model->setHorizontalHeaderItem(0, new QStandardItem("New Column"));
    modelTables.push_back(model);
    QTableView* viewTable = new QTableView(this);
    viewTable->setModel(model);
    addTab(viewTable, "Table1");
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

bool BookView::modified() const
{
    if (isModified)
        return true;
    if (modelTables.count() != 1)
        return true;
    if (modelTables[0]->rowCount() != 1 || modelTables[0]->columnCount() != 1)
        return true;
    return modelTables[0]->item(0, 0) != nullptr;
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

int BookView::getTableViewIndex(int tableID) const
{
    for (int i=0; i<count(); i++)
    {
        if (widget(i)->property(OPENOTE_BOOKVIEW_PROP_TABLE_ID)
                     .toInt() == tableID)
            return i;
    }
    return -1;
}

int BookView::getColumnViewIndex(int tableID, int columnID) const
{
    int tableIndex = getTableIndex(tableID);
    for (int i=0; i<modelTables[i]->columnCount(); i++)
    {
        if (modelTables[tableIndex]->headerData(i, Qt::Horizontal, Qt::UserRole)
                == columnID)
            return i;
    }
    return -1;
}

bool BookView::setColumnHeader(const QString &text, int columnID)
{
    if (count() < 1 || currentWidget() == nullptr)
        return false;

    int tableID =
            currentWidget()->property(OPENOTE_BOOKVIEW_PROP_TABLE_ID).toInt();
    int columnIndex = getColumnViewIndex(tableID, columnID);
    if (columnIndex < 0)
        return false;

    modelTables[getTableIndex(tableID)]
                    ->setHeaderData(columnIndex, Qt::Horizontal, text);
    return true;
}
