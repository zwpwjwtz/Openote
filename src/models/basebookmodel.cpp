#include <algorithm>
#include "basebookmodel.h"
#include "basebookmodel_p.h"


BaseBookModel::BaseBookModel()
    : ONBook (new BaseBookModelPrivate(this))
{
    d = static_cast<BaseBookModelPrivate*>(ONBook::d_ptr);
}

BaseBookModel::BaseBookModel(BaseBookModelPrivate* data)
    : ONBook (data)
{
    d = data;
}

void BaseBookModel::onTableColumnAdded(int tableID,
                                       int columnID,
                                       int referenceID)
{
    if (referenceID > 0)
        ONBook::setColumnReference(tableID, columnID, referenceID);
}

void BaseBookModel::onTableColumnRemoved(int tableID, int columnID)
{
    ONBook::removeColumnReference(tableID, columnID);
}

bool BaseBookModel::load()
{
    if (!ONBook::load())
        return false;

    // Notify all TableModel with column reference IDs
    BaseTableModel* table = nullptr;
    for (auto i=d->columnReference.cbegin(); i!=d->columnReference.cend(); i++)
    {
        if (table == nullptr || table->ID != i->first.first)
        {
            table = static_cast<BaseTableModel*>
                        (d->tableList[d->getTableIndexByID(i->first.first)]);
        }
        table->setColumnReference(table->columnIndex(i->first.second),
                                  i->second);
    }
    return true;
}

BaseTableModel* BaseBookModel::table(int tableIndex) const
{
    if (tableIndex >= 0 && tableIndex < d->tableList.size())
        return static_cast<BaseTableModel*>(d->tableList[tableIndex]);
    else
        return nullptr;
}

BaseTableModel* BaseBookModel::newTable()
{
    return new BaseTableModel;
}

BaseTableModel* BaseBookModel::addTable(const std::string& tableName)
{
    // Assuming increasing ID of tables in the list
    int availableID = d->tableList.size() > 0 ?
                      d->tableList.back()->ID + 1 : 1;

    BaseTableModel* newTable = this->newTable();
    newTable->ID = availableID;
    d->tableList.push_back(newTable);
    d->tableIDList.push_back(availableID);
    d->tableNameList.push_back(tableName);
    newTable->bindBookModel(this);

    return newTable;
}

BaseTableModel* BaseBookModel::duplicateTable(int tableIndex,
                                              const std::string& newName)
{
    const BaseTableModel* table = this->table(tableIndex);
    if (table == nullptr)
        return nullptr;

    // Assuming increasing ID of tables in the list
    int availableID = d->tableList.size() > 0 ?
                      d->tableList.back()->ID + 1 : 1;

    BaseTableModel* newTable = this->newTable();
    newTable->ID = availableID;
    d->tableList.push_back(newTable);
    d->tableIDList.push_back(availableID);
    d->tableNameList.push_back(newName);
    for (auto i=d->columnReference.begin(); i!=d->columnReference.cend(); i++)
    {
        if (i->first.first == table->ID)
        {
            d->columnReference.insert(
                std::make_pair(std::make_pair(newTable->ID, i->first.second),
                               i->second));
        }
    }
    newTable->bindBookModel(this);

    return newTable;
}

bool BaseBookModel::removeTable(int tableIndex)
{
    auto table = this->table(tableIndex);
    if (table == nullptr)
        return false;

    int tableID = table->ID;
    delete d->tableList[tableIndex];
    d->tableList.erase(d->tableList.begin() + tableIndex);
    d->tableIDList.erase(d->tableIDList.begin() + tableIndex);
    d->tableNameList.erase(d->tableNameList.begin() + tableIndex);
    for (auto i=d->columnReference.begin(); i!=d->columnReference.end(); )
    {
        if (i->first.first == tableID)
            i = d->columnReference.erase(i);
        else
            i++;
    }

    // Update the data of parent class
    BaseBookModel::d_ptr->tableList.erase(
                        BaseBookModel::d_ptr->tableList.begin() + tableIndex);

    return true;
}

std::string BaseBookModel::tableName(int tableIndex) const
{
    auto table = this->table(tableIndex);
    if (table != nullptr)
        return d_ptr->tableNameList[tableIndex];
    else
        return std::string();
}

bool BaseBookModel::setTableName(int tableIndex, const std::string& newName)
{
    auto table = this->table(tableIndex);
    if (table != nullptr)
    {
        d_ptr->tableNameList[tableIndex] = newName;
        return true;
    }
    else
        return false;
}

bool BaseBookModel::setColumnReference(int sourceTableIndex,
                                       int sourceColumnIndex,
                                       int targetTableIndex)
{
    auto sourceTable = this->table(sourceTableIndex);
    auto targetTable = this->table(targetTableIndex);
    if (sourceTable == nullptr || targetTable == nullptr)
        return false;

    int sourceColumnID = sourceTable->columnID(sourceColumnIndex);
    if (sourceColumnID > 0)
        return ONBook::setColumnReference(sourceTable->ID,
                                          sourceColumnID,
                                          targetTable->ID);
    else
        return false;
}

void BaseBookModel::removeColumnReference(int tableIndex, int columnIndex)
{
    auto table = this->table(tableIndex);
    if (table == nullptr)
        return;

    int columnID = table->columnID(columnIndex);
    if (columnID > 0)
        ONBook::removeColumnReference(table->ID, columnID);
}

BaseTableModel*
BaseBookModel::convertColumnToTable(BaseTableModel* sourceTable,
                                    int sourceColumnIndex,
                                    const std::string& newTableName)
{
    BaseTableModel* newTable = addTable(newTableName);
    if (newTable == nullptr || newTable->ID <= 0)
        return nullptr;

    // Create an empty column in the new table
    std::string columnName = sourceTable->columnName(sourceColumnIndex);
    BaseTableModel::ColumnType columnType =
                                    sourceTable->columnType(sourceColumnIndex);

    // Get values in the source column, remove duplicated values,
    // then fill the new column by unique values
    int targetColumnID = newTable->newColumn(columnName, columnType);
    std::list<int> IDList = sourceTable->IDs(), newIDList;
    std::map<int, int> indexMap;
    std::list<int>::const_iterator i = IDList.cbegin();
    switch (columnType)
    {
        case BaseTableModel::Integer:
        {
            std::list<int> intList;
            for (; i!=IDList.cend(); i++)
                intList.push_back(sourceTable->readInt(*i, sourceColumnIndex));
            BaseBookModelPrivate::removeDuplicate<int>(intList, indexMap);
            newIDList = newTable->insert(targetColumnID, intList);
            break;
        }
        case BaseTableModel::Double:
        {
            std::list<double> doubleList;
            for (; i!=IDList.cend(); i++)
                doubleList.push_back(
                            sourceTable->readDouble(*i, sourceColumnIndex));
            BaseBookModelPrivate::removeDuplicate<double>(doubleList, indexMap);
            newIDList = newTable->insert(targetColumnID, doubleList);
            break;
        }
        case BaseTableModel::String:
        {
            std::list<std::string> stringList;
            for (; i!=IDList.cend(); i++)
                stringList.push_back(
                            sourceTable->readString(*i, sourceColumnIndex));
            BaseBookModelPrivate::removeDuplicate<std::string>(stringList,
                                                               indexMap);
            newIDList = newTable->insert(targetColumnID, stringList);
            break;
        }
        case BaseTableModel::IntegerList:
        {
            std::list<std::vector<int>> intListList;
            for (; i!=IDList.cend(); i++)
                intListList.push_back(
                             sourceTable->readIntList(*i, sourceColumnIndex));
            BaseBookModelPrivate::removeDuplicate<std::vector<int>>(intListList,
                                                                    indexMap);
            newIDList = newTable->insert(targetColumnID, intListList);
            break;
        }
        default:;
    }

    // Replace the content of the source column with new row IDs
    // Change the column type if needed
    sourceTable->clearColumn(sourceColumnIndex);
    sourceTable->setColumnType(sourceColumnIndex, BaseTableModel::IntegerList);
    int index;
    std::list<int>::const_iterator j;
    for (i=IDList.begin(), index = 0; i!=IDList.end(); i++, index++)
    {
        j = newIDList.cbegin();
        std::advance(j, indexMap[index]);
        sourceTable->modify(*i, sourceColumnIndex, std::vector<int>({ *j }));
    }

    // Update the column reference
    setColumnReference(sourceTable->ID, sourceColumnIndex, newTable->ID);
    sourceTable->setColumnReference(sourceColumnIndex, newTable->ID);

    return newTable;
}

BaseTableModel*
BaseBookModel::columnReferenceTable(int tableID, int columnIndex)
{
    int tableIndex = d_ptr->getTableIndexByID(tableID);
    if (tableIndex < 0)
        return nullptr;

    int sourceColumnID = table(tableIndex)->columnID(columnIndex);
    int targetTableID = columnReference(tableID, sourceColumnID);
    if (targetTableID > 0)
    {
        tableIndex = d_ptr->getTableIndexByID(targetTableID);
        if (tableIndex >= 0)
            return static_cast<BaseTableModel*>(d->tableList[tableIndex]);
    }
    return nullptr;
}

BaseBookModelPrivate::BaseBookModelPrivate(BaseBookModel* parent)
    : ONBookPrivate (parent)
{
    q = static_cast<BaseBookModel*>(q_ptr);
}

bool BaseBookModelPrivate::loadTable(int tableID, const std::string& tableName)
{
    BaseTableModel* table = q->newTable();
    tableList.push_back(table);
    if (!(table->setBindingDirectory(getTableDirectory(tableID)) &&
          table->load()))
        return false;

    table->ID = tableID;
    tableIDList.push_back(tableID);
    tableNameList.push_back(tableName);
    table->bindBookModel(q);

    return true;
}

template <typename T>
void BaseBookModelPrivate::removeDuplicate(std::list<T>& valueList,
                                           std::map<int, int>& indexMap)
{
    // Remove duplicated values in the valueList
    // Build a map from old indexes to new indexes and store it in indexMap
    int index = 0;
    typename std::list<T>::iterator i;
    typename std::list<T>::const_iterator pos;
    for (auto i=valueList.begin(); i!=valueList.end(); )
    {
        pos = std::find(valueList.begin(), i, *i);
        if (pos == i)
        {
            // The current value does not appear before
            // Keep it, and create an item with a new index in the map
            indexMap.insert({index, indexMap.size()});
            i++;
        }
        else
        {
            // The current value has appeared before
            // Remove it from the list, and use the index of its older apperance
            indexMap.insert({index, int(distance(valueList.cbegin(), pos))});
            i = valueList.erase(i);
        }
        index++;
    }
}
