#include <QList>
#include <QMap>
#include <sstream>
#include "bookmodel.h"
#include "bookmodel_p.h"
#include "tablemodel.h"
#include "OpenTable/onbook_p.h"


BookModel::BookModel(QObject *parent) :
    QObject(parent),
    ONBook (new BookModelPrivate)
{
    d = dynamic_cast<BookModelPrivate*>(ONBook::d_ptr);
    d->q_ptr = this;
}

BookModel::~BookModel()
{
    clear();
}

void BookModel::clear()
{
    for (size_t i=0; i<d->tableList.size(); i++)
        delete d->tableList[i];
    d->tableList.clear();
    d->tableIDList.clear();
    d->tableNameList.clear();

    // Update the data of parent class
    ONBook::d_ptr->tableList.clear();
}

int BookModel::tableCount() const
{
    return ONBook::count();
}

QList<int> BookModel::tableIDs() const
{
    auto stdList = ONBook::tableIDs();
    QList<int> IDList;
    IDList.reserve(stdList.size());
    for (auto i=stdList.cbegin(); i!=stdList.cend(); i++)
        IDList.push_back(*i);
    return IDList;
}

QString BookModel::tableName(int tableID) const
{
    return QString::fromStdString(ONBook::tableName(tableID));
}

bool BookModel::setTableName(int tableID, const QString& newName)
{
    return ONBook::setTableName(tableID, newName.toStdString());
}

TableModel* BookModel::table(int tableID) const
{
    return dynamic_cast<TableModel*>(ONBook::table(tableID));
}

TableModel* BookModel::addTable(const QString& tableName)
{
    // Assuming increasing ID of tables in the list
    int availableID = d->tableList.size() > 0 ?
                      d->tableList.back()->ID + 1 : 1;

    TableModel* newTable = new TableModel(this);
    newTable->ID = availableID;
    d->tableList.push_back(newTable);
    d->tableIDList.push_back(availableID);
    d->tableNameList.push_back(tableName.toStdString());
    connect(newTable, SIGNAL(columnAdded(int, int, int)),
            this, SLOT(onTableColumnAdded(int, int, int)));
    connect(newTable, SIGNAL(columnRemoved(int, int)),
            this, SLOT(onTableColumnRemoved(int, int)));

    // Update the data of parent class
    ONBook::d_ptr->tableList.push_back(newTable);

    return newTable;
}

TableModel* BookModel::duplicateTable(int tableID, const QString& newName)
{
    const TableModel* table = this->table(tableID);
    if (table == nullptr)
        return nullptr;

    // Assuming increasing ID of tables in the list
    int availableID = d->tableList.size() > 0 ?
                      d->tableList.back()->ID + 1 : 1;

    TableModel* newTable = new TableModel(*table);
    newTable->ID = availableID;
    d->tableList.push_back(newTable);
    d->tableIDList.push_back(availableID);
    d->tableNameList.push_back(newName.toStdString());
    for (auto i=d->columnReference.begin(); i!=d->columnReference.cend(); i++)
    {
        if (i->first.first == tableID)
        {
            d->columnReference.insert(
                std::make_pair(std::make_pair(newTable->ID, i->first.second),
                               i->second));
        }
    }
    connect(newTable, SIGNAL(columnAdded(int, int, int)),
            this, SLOT(onTableColumnAdded(int, int, int)));
    connect(newTable, SIGNAL(columnRemoved(int, int)),
            this, SLOT(onTableColumnRemoved(int, int)));

    // Update the data of parent class
    ONBook::d_ptr->tableList.push_back(newTable);

    return newTable;
}

bool BookModel::removeTable(int tableID)
{
    int index = d->getTableIndexByID(tableID);
    if (index < 0)
        return false;

    delete d->tableList[index];
    d->tableList.erase(d->tableList.begin() + index);
    d->tableIDList.erase(d->tableIDList.begin() + index);
    d->tableNameList.erase(d->tableNameList.begin() + index);
    for (auto i=d->columnReference.begin(); i!=d->columnReference.end(); )
    {
        if (i->first.first == tableID)
            i = d->columnReference.erase(i);
        else
            i++;
    }

    // Update the data of parent class
    ONBook::d_ptr->tableList.erase(ONBook::d_ptr->tableList.begin() + index);

    return true;
}

TableModel* BookModel::convertColumnToTable(TableModel* sourceTable,
                                            int sourceColumnID,
                                            const QString& newTableName)
{
    TableModel* newTable = addTable(newTableName);
    if (newTable == nullptr || newTable->ID <= 0)
        return nullptr;

    // Create an empty column in the new table
    std::string columnName = sourceTable->columnName(sourceColumnID);
    TableModel::ColumnType columnType = sourceTable->columnType(sourceColumnID);

    // Get values in the source column, remove duplicated values,
    // then fill the new column by unique values
    int targetColumnID = newTable->newColumn(columnName, columnType);
    std::list<int> IDList = sourceTable->IDs(), newIDList;
    QMap<int, int> indexMap;
    std::list<int>::const_iterator i = IDList.cbegin();
    switch (columnType)
    {
        case TableModel::Integer:
        {
            std::list<int> intList;
            for (; i!=IDList.cend(); i++)
                intList.push_back(sourceTable->readInt(*i, sourceColumnID));
            BookModelPrivate::removeDuplicate<int>(intList, indexMap);
            newIDList = newTable->insert(targetColumnID, intList);
            break;
        }
        case TableModel::Double:
        {
            std::list<double> doubleList;
            for (; i!=IDList.cend(); i++)
                doubleList.push_back(
                            sourceTable->readDouble(*i, sourceColumnID));
            BookModelPrivate::removeDuplicate<double>(doubleList, indexMap);
            newIDList = newTable->insert(targetColumnID, doubleList);
            break;
        }
        case TableModel::String:
        {
            std::list<std::string> stringList;
            for (; i!=IDList.cend(); i++)
                stringList.push_back(
                            sourceTable->readString(*i, sourceColumnID));
            BookModelPrivate::removeDuplicate<std::string>(stringList,
                                                           indexMap);
            newIDList = newTable->insert(targetColumnID, stringList);
            break;
        }
        case TableModel::IntegerList:
        {
            std::list<std::vector<int>> intListList;
            for (; i!=IDList.cend(); i++)
                intListList.push_back(
                             sourceTable->readIntList(*i, sourceColumnID));
            BookModelPrivate::removeDuplicate<std::vector<int>>(intListList,
                                                                indexMap);
            newIDList = newTable->insert(targetColumnID, intListList);
            break;
        }
        default:;
    }

    // Replace the content of the source column with new row IDs
    // Change the column type if needed
    sourceTable->clearColumn(sourceColumnID);
    sourceTable->setColumnType(sourceColumnID, TableModel::IntegerList);
    int index;
    std::list<int>::const_iterator j;
    for (i=IDList.begin(), index = 0; i!=IDList.end(); i++, index++)
    {
        j = newIDList.cbegin();
        std::advance(j, indexMap[index]);
        sourceTable->modify(*i, sourceColumnID, std::vector<int>({ *j }));
    }

    // Update the column reference
    setColumnReference(sourceTable->ID, sourceColumnID, newTable->ID);
    sourceTable->setColumnReference(sourceColumnID, newTable->ID);

    return newTable;
}

TableModel*
BookModel::columnReferenceTable(int sourceTableID, int sourceColumnID)
{
    int targetTableID = ONBook::columnReference(sourceTableID, sourceColumnID);
    if (targetTableID > 0)
    {
        int index = d->getTableIndexByID(targetTableID);
        if (index >= 0)
            return d->tableList[index];
    }
    return nullptr;
}

QString BookModel::path() const
{
    return QString::fromStdString(ONBook::bindingDirectory());
}

bool BookModel::setPath(const QString& path)
{
    if (!path.isEmpty())
        return ONBook::setBindingDirectory(path.toStdString());
    ONBook::clearBindingDirectory();
    return true;
}

bool BookModel::load()
{
    if (!ONBook::load())
        return false;

    // Listen to tables' events
    for (auto j=d->tableList.cbegin(); j!=d->tableList.cend(); j++)
    {
        connect(*j, SIGNAL(columnAdded(int, int, int)),
                this, SLOT(onTableColumnAdded(int, int, int)));
        connect(*j, SIGNAL(columnRemoved(int, int)),
                this, SLOT(onTableColumnRemoved(int, int)));
    }

    // Notify all TableModel with column reference IDs
    TableModel* table = nullptr;
    for (auto i=d->columnReference.cbegin(); i!=d->columnReference.cend(); i++)
    {
        if (table == nullptr || table->ID != i->first.first)
        {
            table = d->tableList[d->getTableIndexByID(i->first.first)];
        }
        table->setColumnReference(i->first.second, i->second);
    }
    return true;
}

bool BookModel::save()
{
    return ONBook::save();
}

BookIndex BookModel::find(QString text, BookIndex startIndex,
                          bool forward, bool inAllTables, bool caseSensitive)
{
    BookIndex searchIndex = startIndex;

    // Find text in the following order: row->column->table
    bool found = false;
    QList<int> tableIDList = tableIDs();
    TableModel* currentTable = table(tableIDList[searchIndex.table]);
    QModelIndex tableIndex;
    int rowCount = currentTable->countRow();
    int columnCount = currentTable->countColumn();
    Qt::CaseSensitivity caseSensitivity = caseSensitive ?
                                          Qt::CaseSensitivity::CaseSensitive :
                                          Qt::CaseSensitivity::CaseInsensitive;
    while (true)
    {
        if (forward)
        {
            // Advance to the next row
            searchIndex.row++;
            if (searchIndex.row >= rowCount)
            {
                // Step into the next column
                searchIndex.row = 0;
                searchIndex.column++;
            }
            if (searchIndex.column >= columnCount)
            {
                searchIndex.column = 0;
                if (inAllTables)
                {
                    // Step into the next table
                    searchIndex.table++;
                    if (searchIndex.table >= tableCount())
                        searchIndex.table = 0;

                    currentTable = table(tableIDList[searchIndex.table]);
                    rowCount = currentTable->countRow();
                    columnCount = currentTable->countColumn();
                }
            }
        }
        else
        {
            // Regress to the previous row
            searchIndex.row--;
            if (searchIndex.row < 0)
            {
                // Step back to the previous column
                searchIndex.row = rowCount - 1;
                searchIndex.column--;
            }
            if (searchIndex.column < 0)
            {
                searchIndex.column = columnCount - 1;
                if (inAllTables)
                {
                    // Step back to the previous table
                    searchIndex.table++;
                    if (searchIndex.table >= tableCount())
                        searchIndex.table = 0;

                    currentTable = table(tableIDList[searchIndex.table]);
                    rowCount = currentTable->countRow();
                    columnCount = currentTable->countColumn();

                    searchIndex.column = columnCount - 1;
                    searchIndex.row = rowCount - 1;
                }
            }
        }

        // Compare the displayed content of the grid with searched text
        tableIndex = currentTable->index(searchIndex.row, searchIndex.column);
        if (currentTable->data(tableIndex, Qt::DisplayRole)
                        .toString().contains(text, caseSensitivity))
        {
            found = true;
            break;
        }

        if (searchIndex == startIndex)
        {
            // Reach the starting position: no match found
            searchIndex.reset();
            break;
        }
    }

    return searchIndex;
}

void BookModel::onTableColumnAdded(int tableID, int columnID, int referenceID)
{
    if (referenceID > 0)
        ONBook::setColumnReference(tableID, columnID, referenceID);
}

void BookModel::onTableColumnRemoved(int tableID, int columnID)
{
    ONBook::removeColumnReference(tableID, columnID);
}

bool BookModelPrivate::loadTable(int tableID, const std::string& tableName)
{
    TableModel* table = new TableModel(q_ptr);
    tableList.push_back(table);
    if (!(table->setBindingDirectory(getTableDirectory(tableID)) &&
          table->load()))
        return false;

    table->ID = tableID;
    tableIDList.push_back(tableID);
    tableNameList.push_back(tableName);

    // Update the data of parent class
    ONBookPrivate::tableList.push_back(table);

    return true;
}

template <typename T>
void BookModelPrivate::removeDuplicate(std::list<T>& valueList,
                                       QMap<int, int>& indexMap)
{
    int index = 0;
    typename std::list<T>::iterator i;
    typename std::list<T>::const_iterator pos;
    for (auto i=valueList.begin(); i!=valueList.end(); )
    {
        pos = std::find(valueList.begin(), i, *i);
        if (pos == i)
        {
            indexMap.insert(index, indexMap.count());
            i++;
        }
        else
        {
            indexMap.insert(index, int(distance(valueList.cbegin(), pos)));
            i = valueList.erase(i);
        }
        index++;
    }
}
