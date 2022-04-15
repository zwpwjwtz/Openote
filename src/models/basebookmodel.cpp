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

BaseTableModel* BaseBookModel::newBaseTable(const BaseTableModel* src)
{
    if (src == nullptr)
        return new BaseTableModel();
    else
        return new BaseTableModel(*src);
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
        table->setColumnReference(i->first.second, i->second);
    }
    return true;
}

int BaseBookModel::tableCount() const
{
    return count();
}

std::vector<int> BaseBookModel::tableIDs() const
{
    auto stdList = ONBook::tableIDs();
    std::vector<int> IDList;
    IDList.reserve(stdList.size());
    for (auto i=stdList.cbegin(); i!=stdList.cend(); i++)
        IDList.push_back(*i);
    return IDList;
}

BaseTableModel* BaseBookModel::table(int tableID) const
{
    return static_cast<BaseTableModel*>(ONBook::table(tableID));
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

    BaseTableModel* newTable = newBaseTable(nullptr);
    newTable->ID = availableID;
    d->tableList.push_back(newTable);
    d->tableIDList.push_back(availableID);
    d->tableNameList.push_back(tableName);
    newTable->bindBookModel(this);

    return newTable;
}

BaseTableModel* BaseBookModel::duplicateTable(int tableID,
                                              const std::string& newName)
{
    const BaseTableModel* table = this->table(tableID);
    if (table == nullptr)
        return nullptr;

    // Assuming increasing ID of tables in the list
    int availableID = d->tableList.size() > 0 ?
                      d->tableList.back()->ID + 1 : 1;

    BaseTableModel* newTable = newBaseTable(table);
    newTable->ID = availableID;
    d->tableList.push_back(newTable);
    d->tableIDList.push_back(availableID);
    d->tableNameList.push_back(newName);
    for (auto i=d->columnReference.begin(); i!=d->columnReference.cend(); i++)
    {
        if (i->first.first == tableID)
        {
            d->columnReference.insert(
                std::make_pair(std::make_pair(newTable->ID, i->first.second),
                               i->second));
        }
    }
    newTable->bindBookModel(this);

    return newTable;
}

bool BaseBookModel::removeTable(int tableID)
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
    BaseBookModel::d_ptr->tableList.erase(
                            BaseBookModel::d_ptr->tableList.begin() + index);

    return true;
}

BaseTableModel*
BaseBookModel::convertColumnToTable(BaseTableModel* sourceTable,
                                    int sourceColumnID,
                                    const std::string& newTableName)
{
    BaseTableModel* newTable = addTable(newTableName);
    if (newTable == nullptr || newTable->ID <= 0)
        return nullptr;

    // Create an empty column in the new table
    std::string columnName = sourceTable->columnName(sourceColumnID);
    BaseTableModel::ColumnType columnType =
                                    sourceTable->columnType(sourceColumnID);

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
                intList.push_back(sourceTable->readInt(*i, sourceColumnID));
            BaseBookModelPrivate::removeDuplicate<int>(intList, indexMap);
            newIDList = newTable->insert(targetColumnID, intList);
            break;
        }
        case BaseTableModel::Double:
        {
            std::list<double> doubleList;
            for (; i!=IDList.cend(); i++)
                doubleList.push_back(
                            sourceTable->readDouble(*i, sourceColumnID));
            BaseBookModelPrivate::removeDuplicate<double>(doubleList, indexMap);
            newIDList = newTable->insert(targetColumnID, doubleList);
            break;
        }
        case BaseTableModel::String:
        {
            std::list<std::string> stringList;
            for (; i!=IDList.cend(); i++)
                stringList.push_back(
                            sourceTable->readString(*i, sourceColumnID));
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
                             sourceTable->readIntList(*i, sourceColumnID));
            BaseBookModelPrivate::removeDuplicate<std::vector<int>>(intListList,
                                                                    indexMap);
            newIDList = newTable->insert(targetColumnID, intListList);
            break;
        }
        default:;
    }

    // Replace the content of the source column with new row IDs
    // Change the column type if needed
    sourceTable->clearColumn(sourceColumnID);
    sourceTable->setColumnType(sourceColumnID, BaseTableModel::IntegerList);
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

BaseTableModel*
BaseBookModel::columnReferenceTable(int sourceTableID, int sourceColumnID)
{
    int targetTableID = BaseBookModel::columnReference(sourceTableID,
                                                       sourceColumnID);
    if (targetTableID > 0)
    {
        int index = d->getTableIndexByID(targetTableID);
        if (index >= 0)
            return static_cast<BaseTableModel*>(d->tableList[index]);
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
    BaseTableModel* table = q->newBaseTable(nullptr);
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
