#include "basetablemodel.h"
#include "basetablemodel_p.h"
#include "basebookmodel.h"
#include "OpenTable/ontableintcolumn.h"
#include "OpenTable/ontabledoublecolumn.h"
#include "OpenTable/ontablestringcolumn.h"
#include "OpenTable/ontableintlistcolumn.h"


BaseTableModel::BaseTableModel()
    : ONTable (new BaseTableModelPrivate())
{
    d = static_cast<BaseTableModelPrivate*>(ONTable::d_ptr);
}

BaseTableModel::BaseTableModel(const BaseTableModel& src)
    : ONTable (new BaseTableModelPrivate(*src.d))
{
    d = static_cast<BaseTableModelPrivate*>(ONTable::d_ptr);
}

BaseTableModel::BaseTableModel(BaseTableModelPrivate* data)
    : ONTable (data)
{
    d = data;
}

int BaseTableModel::rowID(int rowIndex) const
{
    if (rowIndex < 0)
        return 0;

    auto i = d->IDList.cbegin();
    std::advance(i, rowIndex);
    if (i == d->IDList.cend())
        return 0;
    else
        return *i;
}

int BaseTableModel::columnID(int columnIndex) const
{
    if (columnIndex >= 0 && columnIndex < d->columnIDList.size())
        return d->columnIDList[columnIndex];
    else
        return 0;
}

int BaseTableModel::columnIndex(int columnID) const
{
    return d->getColumnIndexByID(columnID);
}

bool BaseTableModel::bindBookModel(BaseBookModel* model)
{
    d->book = model;
    return true;
}

bool BaseTableModel::setColumnReference(int columnIndex, int referenceID)
{
    if (columnIndex < 0 || columnIndex > d->columnReferenceIDList.size())
        return false;

    if (columnIndex == d->columnReferenceIDList.size())
        d->columnReferenceIDList.push_back(referenceID);
    else
        d->columnReferenceIDList[columnIndex] = referenceID;

    // Inform the parent book about the reference
    ONBook* book = static_cast<ONBook*>(d->book);
    if (referenceID > 0)
        book->setColumnReference(ID, d->columnIDList[columnIndex], referenceID);
    else
        book->removeColumnReference(ID, d->columnIDList[columnIndex]);
    return true;
}

void BaseTableModel::clear(int rowIndex, int columnIndex)
{
    ONTable::clear(rowID(rowIndex), columnID(columnIndex));
}

void BaseTableModel::clearRow(int rowIndex)
{
    ONTable::clearRow(rowID(rowIndex));
}

void BaseTableModel::clearColumn(int columnIndex)
{
    ONTable::clearColumn(columnID(columnIndex));
}

int BaseTableModel::newRow()
{
    if (ONTable::newRow() > 0)
        return countRow() - 1;
    else
        return -1;
}

int BaseTableModel::newColumn(const std::string& name,
                              ColumnType columnType,
                              int referenceID)
{
    if (ONTable::newColumn(name, columnType) > 0)
    {
        int newIndex = countColumn() - 1;
        setColumnReference(newIndex, referenceID);
        return newIndex;
    }
    else
        return -1;
}

int BaseTableModel::insertRow(int rowIndex)
{
    if (ONTable::newRow() > 0)
    {
        moveRow(countRow() - 1, rowIndex);
        return rowIndex;
    }
    else
        return -1;
}

int BaseTableModel::insertColumn(int columnIndex,
                                 const std::string& name,
                                 ColumnType columnType,
                                 int referenceID)
{
    if (newColumn(name, columnType, referenceID) > 0)
    {
        moveColumn(countColumn() - 1, columnIndex);
        return columnIndex;
    }
    else
        return -1;
}

bool BaseTableModel::duplicateRow(int rowIndex)
{
    int oldRowID = rowID(rowIndex);
    int newRowID = ONTable::newRow();
    int columnCount = d->columnList.size();
    for (int i=0; i<columnCount; i++)
    {
        ONTableColumn* column = d->columnList[i];
        switch (d->columnTypeIDList[i])
        {
            case ColumnType::Integer:
                ((ONTableIntColumn*)column)->duplicate(oldRowID, newRowID);
                break;
            case ColumnType::Double:
                ((ONTableDoubleColumn*)column)->duplicate(oldRowID, newRowID);
                break;
            case ColumnType::String:
                ((ONTableStringColumn*)column)->duplicate(oldRowID, newRowID);
                break;
            case ColumnType::IntegerList:
                ((ONTableIntListColumn*)column)->duplicate(oldRowID, newRowID);
                break;
            default:;
        }
    }
    return true;
}

bool BaseTableModel::duplicateColumn(int columnIndex,
                                     const std::string& newName)
{
    ONTableColumn* oldColumn = ONTable::d_ptr->columnList[columnIndex];
    ONTableColumn* newColumn;
    switch (oldColumn->typeID)
    {
        case ColumnType::Integer:
            newColumn = new ONTableIntColumn(
                            *(ONTableIntColumn*)oldColumn);
            break;
        case ColumnType::Double:
            newColumn = new ONTableDoubleColumn(
                            *(ONTableDoubleColumn*)oldColumn);
            break;
        case ColumnType::String:
            newColumn = new ONTableStringColumn(
                            *(ONTableStringColumn*)oldColumn);
            break;
        case ColumnType::IntegerList:
            newColumn = new ONTableIntListColumn(
                            *(ONTableIntListColumn*)oldColumn);
            break;
        default:
            newColumn = nullptr;
            return false;
    }

    // Assuming increasing ID of column in the list
    int availableID = d->columnList.size() > 0 ?
                      d->columnList.back()->ID + 1 :
                      1;
    newColumn->ID = availableID;
    d->columnList.push_back(newColumn);
    d->columnIDList.push_back(availableID);
    d->columnTypeIDList.push_back(newColumn->typeID);
    d->columnNameList.push_back(newName);
    d->columnReferenceIDList.push_back(d->columnReferenceIDList[columnIndex]);
    if (d->book != nullptr)
        d->book->onTableColumnAdded(ID, newColumn->ID,
                                    d->columnReferenceIDList[columnIndex]);

    return true;
}

void BaseTableModel::moveRow(int fromIndex, int toIndex)
{
    if (fromIndex < 0 || fromIndex >= d->IDList.size() ||
        toIndex < 0 || toIndex > d->IDList.size() ||
        fromIndex == toIndex)
        return;

    auto i = d->IDList.begin();
    std::advance(i, fromIndex);
    if (toIndex == d->IDList.size())
    {
        d->IDList.push_back(*i);
    }
    else
    {
        auto j = d->IDList.begin();
        std::advance(j, toIndex);
        d->IDList.insert(j, *i);
    }
    d->IDList.erase(i);
}

void BaseTableModel::moveColumn(int fromIndex, int toIndex)
{
    if (fromIndex < 0 || fromIndex >= d->columnIDList.size() ||
        toIndex < 0 || toIndex > d->columnIDList.size() ||
        fromIndex == toIndex)
        return;

    int columnID = d->columnIDList[fromIndex];
    int columnTypeID = d->columnTypeIDList[fromIndex];
    int columnReferenceID = d->columnReferenceIDList[fromIndex];
    std::string columnName = d->columnNameList[fromIndex];
    ONTableColumn* column = d->columnList[fromIndex];
    if (fromIndex < toIndex)
    {
        for (int i = fromIndex; i<toIndex; i++)
        {
            d->columnIDList[i] = d->columnIDList[i + 1];
            d->columnTypeIDList[i] = d->columnTypeIDList[i + 1];
            d->columnReferenceIDList[i] = d->columnReferenceIDList[i + 1];
            d->columnNameList[i] = d->columnNameList[i + 1];
            d->columnList[i] = d->columnList[i + 1];
        }
    }
    else
    {
        for (int i = fromIndex; i>toIndex; i--)
        {
            d->columnIDList[i] = d->columnIDList[i - 1];
            d->columnTypeIDList[i] = d->columnTypeIDList[i - 1];
            d->columnReferenceIDList[i] = d->columnReferenceIDList[i - 1];
            d->columnNameList[i] = d->columnNameList[i - 1];
            d->columnList[i] = d->columnList[i - 1];
        }
    }
    d->columnIDList[toIndex] = columnID;
    d->columnTypeIDList[toIndex] = columnTypeID;
    d->columnReferenceIDList[toIndex] = columnReferenceID;
    d->columnNameList[toIndex] = columnName;
    d->columnList[toIndex] = column;
}

void BaseTableModel::removeRow(int rowIndex)
{
    ONTable::removeRow(rowID(rowIndex));
}

void BaseTableModel::removeColumn(int columnIndex)
{
    int columnID = BaseTableModel::d_ptr->columnIDList[columnIndex];
    ONTable::removeColumn(columnID);
    d->columnReferenceIDList.erase(
                        d->columnReferenceIDList.begin() + columnIndex);
    if (d->book != nullptr)
        d->book->onTableColumnRemoved(ID, columnID);
}

std::string BaseTableModel::columnName(int columnIndex) const
{
    return ONTable::columnName(columnID(columnIndex));
}

void BaseTableModel::setColumnName(int columnIndex, const std::string& newName)
{
    ONTable::setColumnName(columnID(columnIndex), newName);
}

ONTable::ColumnType BaseTableModel::columnType(int columnIndex) const
{
    return ONTable::columnType(columnID(columnIndex));
}

bool BaseTableModel::setColumnType(int columnIndex, ColumnType newType)
{
    return ONTable::setColumnType(columnID(columnIndex), newType);
}

bool BaseTableModel::load()
{
    if (!ONTable::load())
        return false;

    // Initialize columnReferenceIDList with zeros
    // These placeholders are necessary for the following
    // loading process of BookModel, because only columns
    // with reference can update their reference IDs in this list
    int count = d->columnList.size();
    d->columnReferenceIDList.reserve(count);
    while (count-- > 0)
        d->columnReferenceIDList.push_back(0);
    return true;
}

int BaseTableModel::readInt(int rowIndex, int columnIndex) const
{
    return ONTable::readInt(rowID(rowIndex), columnID(columnIndex));
}

double BaseTableModel::readDouble(int rowIndex, int columnIndex) const
{
    return ONTable::readDouble(rowID(rowIndex), columnID(columnIndex));
}

std::string BaseTableModel::readString(int rowIndex, int columnIndex) const
{
    return ONTable::readString(rowID(rowIndex), columnID(columnIndex));
}

std::vector<int> BaseTableModel::readIntList(int rowIndex,
                                             int columnIndex) const
{
    return ONTable::readIntList(rowID(rowIndex), columnID(columnIndex));
}

template<typename T>
bool BaseTableModel::modify(int rowIndex, int columnIndex, const T& value)
{
    return ONTable::modify(rowID(rowIndex), columnID(columnIndex), value);
}

template bool BaseTableModel::modify(int, int, const int&);
template bool BaseTableModel::modify(int, int, const double&);
template bool BaseTableModel::modify(int, int, const std::string&);
template bool BaseTableModel::modify(int, int, const std::vector<int>&);

template<typename T>
std::list<int> BaseTableModel::insert(int columnIndex,
                                      const std::list<T>& valueList)
{
    return ONTable::insert(columnIndex, valueList);
}

template std::list<int> BaseTableModel::insert(int, const std::list<int>&);
template std::list<int> BaseTableModel::insert(int, const std::list<double>&);
template
std::list<int> BaseTableModel::insert(int, const std::list<std::string>&);
template
std::list<int> BaseTableModel::insert(int, const std::list<std::vector<int>>&);


BaseTableModelPrivate::BaseTableModelPrivate()
{
    book = nullptr;
}

BaseTableModelPrivate::BaseTableModelPrivate(const BaseTableModelPrivate& src) :
    ONTablePrivate (src)
{
    book = src.book;
    columnReferenceIDList = src.columnReferenceIDList;
}

BaseTableModelPrivate::~BaseTableModelPrivate()
{}
