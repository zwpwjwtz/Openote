#include <algorithm>
#include "ontable.h"
#include "ontable_p.h"
#include "ontableintcolumn.h"
#include "ontabledoublecolumn.h"
#include "ontablestringcolumn.h"


ONTable::ONTable()
{
    d_ptr = new ONTablePrivate();
}

ONTable::ONTable(ONTablePrivate* data)
{
    if (data == nullptr)
        ONTable();
    else
        d_ptr = data;
}

ONTable::~ONTable()
{
    delete d_ptr;
}

int ONTable::ID()
{
    return d_ptr->ID;
}

void ONTable::setID(int ID)
{
    d_ptr->ID = ID;
}

std::string ONTable::name()
{
    return d_ptr->name;
}

void ONTable::setName(const std::string& name)
{
    d_ptr->name = name;
}

int ONTable::countRow()
{
    return d_ptr->IDList.size();
}

int ONTable::countColumn()
{
    return d_ptr->columnList.size();
}

bool ONTable::existsRow(int ID)
{
    return std::find(d_ptr->IDList.cbegin(),
                     d_ptr->IDList.cend(),
                     ID) != d_ptr->IDList.cend();
}

bool ONTable::existsColumn(int ID)
{
    return std::find(d_ptr->columnIDList.cbegin(),
                     d_ptr->columnIDList.cend(),
                     ID) != d_ptr->columnIDList.cend();
}

std::list<int> ONTable::IDs()
{
    return d_ptr->IDList;
}

std::vector<std::string> ONTable::columnNames()
{
    return d_ptr->columnNameList;
}

void ONTable::clear()
{
    std::vector<ONTableColumn>& columns = d_ptr->columnList;
    for (size_t i=0; i<columns.size(); i++)
        columns[i].clear();
    d_ptr->IDList.clear();
}

int ONTable::newRow()
{
    // Assuming increasing ID of row in the list
    int availableID = d_ptr->IDList.back() + 1;
    for (size_t i=0; i<d_ptr->columnList.size(); i++)
        d_ptr->columnList[i].set(availableID, nullptr);
    d_ptr->IDList.push_back(availableID);
    return availableID;
}

int ONTable::newColumn(const std::string& name, ColumnType columnType)
{
    // Assuming increasing ID of column in the list
    int availableID = d_ptr->columnList.size() > 0 ?
                      d_ptr->columnList.back().ID + 1 :
                      1;

    ONTableColumn* emptyColumn;
    switch (columnType)
    {
        case Integer:
            emptyColumn = new ONTableIntColumn;
            break;
        case Double:
            emptyColumn = new ONTableDoubleColumn;
            break;
        case String:
            emptyColumn = new ONTableStringColumn;
            break;
        default:
            return 0;
    }
    emptyColumn->ID = availableID;
    emptyColumn->typeID = columnType;
    d_ptr->columnList.push_back(*emptyColumn);
    d_ptr->columnIDList.push_back(availableID);
    d_ptr->columnNameList.push_back(name);
    delete emptyColumn;

    return availableID;
}

bool ONTable::modify(int ID, int columnID, const int& value)
{
    size_t columnIndex = d_ptr->getColumnIndexByID(columnID);
#ifdef ONTABLE_COLUMN_TYPE_CHECK
    if (d_ptr->columnList[columnIndex].typeID() != ColumnType::Integer)
        return false;
#endif
    dynamic_cast<ONTableIntColumn&>
            (d_ptr->columnList[columnIndex]).set(ID, value);
    return true;
}

bool ONTable::modify(int ID, int columnID, const double& value)
{
    size_t columnIndex = d_ptr->getColumnIndexByID(columnID);
#ifdef ONTABLE_COLUMN_TYPE_CHECK
    if (d_ptr->columnList[columnIndex].typeID() != ColumnType::Double)
        return false;
#endif
    dynamic_cast<ONTableDoubleColumn&>
            (d_ptr->columnList[columnIndex]).set(ID, value);
    return true;
}

bool ONTable::modify(int ID, int columnID, const std::string& value)
{
    size_t columnIndex = d_ptr->getColumnIndexByID(columnID);
#ifdef ONTABLE_COLUMN_TYPE_CHECK
    if (d_ptr->columnList[columnIndex].typeID() != ColumnType::String)
        return false;
#endif
    dynamic_cast<ONTableStringColumn&>
            (d_ptr->columnList[columnIndex]).set(ID, value);
    return true;
}

void ONTable::removeRow(int ID)
{
    std::list<int>::iterator pos = std::find(d_ptr->IDList.begin(),
                                             d_ptr->IDList.end(),
                                             ID);
    if (pos == d_ptr->IDList.end())
        return;
    d_ptr->IDList.erase(pos);
    for (size_t i=0; i<d_ptr->columnList.size(); i++)
       d_ptr->columnList[i].remove(ID);
}

void ONTable::removeColumn(int columnID)
{
    std::vector<ONTableColumn>::iterator i;
    for (i=d_ptr->columnList.begin(); i!=d_ptr->columnList.end(); i++)
        if ((*i).ID == columnID)
            d_ptr->columnList.erase(i);
}

ONTablePrivate::ONTablePrivate()
{
    ID = 0;
}

size_t ONTablePrivate::getColumnIndexByID(int columnID)
{
    for (size_t i=0; i<columnIDList.size(); i++)
    {
        if (columnIDList[i] == columnID)
            return i;
    }
    return -1;
}
