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

int BaseTableModel::rowID(int row) const
{
    return d->getRowID(row);
}

int BaseTableModel::columnID(int column) const
{
    return d->columnIDList[column];
}

bool BaseTableModel::bindBookModel(BaseBookModel* model)
{
    d->book = model;
}

bool BaseTableModel::setColumnReference(int columnID, int referenceID)
{
    int columnIndex = d->getColumnIndexByID(columnID);
    if (columnIndex < 0)
        return false;

    d->columnReferenceIDList[columnIndex] = referenceID;
    return true;
}
bool BaseTableModel::duplicateRow(int row)
{
    int oldRowID = d->getRowID(row);
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

bool BaseTableModel::duplicateColumn(int column, const std::string& newName)
{
    int columnIndex = countColumn();
    ONTableColumn* oldColumn = ONTable::d_ptr->columnList[column];
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
    d->columnReferenceIDList.push_back(d->columnReferenceIDList[column]);
    d->book->onTableColumnAdded(ID, newColumn->ID, d->columnReferenceIDList[column]);

    return true;
}

void BaseTableModel::removeColumn(int column)
{
    int columnID = BaseTableModel::d_ptr->columnIDList[column];
    BaseTableModel::removeColumn(columnID);
    d->columnReferenceIDList.erase(d->columnReferenceIDList.begin() + column);
    d->book->onTableColumnRemoved(ID, columnID);
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

int BaseTableModelPrivate::getRowID(int rowIndex) const
{
    // The order of rows are fixed and does not allow changes
    // So just iterate over the IDList and take the ID located
    // at index rowIndex would be fine
    int count = 0;
    auto i = IDList.cbegin();
    while (i != IDList.cend())
    {
        if (count == rowIndex)
            break;
        count++;
        i++;
    }
    if (i == IDList.cend())
        return 0;
    else
        return *i;
}
