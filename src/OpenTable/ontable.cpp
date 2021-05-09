#include <algorithm>
#include <cstring>
#include <sstream>
#include "ontable.h"
#include "ontable_p.h"
#include "ontableintcolumn.h"
#include "ontabledoublecolumn.h"
#include "ontablestringcolumn.h"
#include "ontableintlistcolumn.h"
#include "utils/filesystem.h"

#define ONTABLE_TABLE_INDEX_FILE_NAME      "ontable-index"
#define ONTABLE_TABLE_INDEX_FILE_HEADER    "ONTABLEINDEX"
#define ONTABLE_TABLE_INDEX_FILE_VERSION   "1.0\n"

#define ONTABLE_TABLE_IO_BUFFER_MAXLEN     256


ONTable::ONTable()
{
    d_ptr = new ONTablePrivate();

    d_ptr->defaultIntValue = 0;
    d_ptr->defaultDoubleValue = 0.0;
}

ONTable::ONTable(const ONTable& src)
{
    d_ptr = new ONTablePrivate(*(src.d_ptr));

    ID = src.ID;
    d_ptr->defaultIntValue = src.d_ptr->defaultIntValue;
    d_ptr->defaultDoubleValue = src.d_ptr->defaultDoubleValue;
    d_ptr->defaultStringValue = src.d_ptr->defaultStringValue;
}

ONTable::ONTable(ONTablePrivate* data)
{
    if (data == nullptr)
        ONTable();
    else
        d_ptr = data;

    d_ptr->defaultIntValue = 0;
    d_ptr->defaultDoubleValue = 0.0;
}

ONTable::~ONTable()
{
    delete d_ptr;
}

int ONTable::countRow() const
{
    return d_ptr->IDList.size();
}

int ONTable::countColumn() const
{
    return d_ptr->columnList.size();
}

bool ONTable::existsRow(int ID) const
{
    return std::find(d_ptr->IDList.cbegin(),
                     d_ptr->IDList.cend(),
                     ID) != d_ptr->IDList.cend();
}

bool ONTable::existsColumn(int ID) const
{
    return std::find(d_ptr->columnIDList.cbegin(),
                     d_ptr->columnIDList.cend(),
                     ID) != d_ptr->columnIDList.cend();
}

std::list<int> ONTable::IDs() const
{
    return d_ptr->IDList;
}

std::vector<int> ONTable::columnIDs() const
{
    return d_ptr->columnIDList;
}

std::string ONTable::columnName(int columnID) const
{
    return d_ptr->columnNameList[d_ptr->getColumnIndexByID(columnID)];
}

void ONTable::setColumnName(int columnID, const std::string& newName)
{
    int index = d_ptr->getColumnIndexByID(columnID);
    if (index >= 0)
        d_ptr->columnNameList[index] = newName;
}

void ONTable::clear()
{
    std::vector<ONTableColumn*>& columns = d_ptr->columnList;
    for (size_t i=0; i<columns.size(); i++)
    {
        columns[i]->clear();
        delete columns[i];
    }
    columns.clear();
    d_ptr->columnIDList.clear();
    d_ptr->columnTypeIDList.clear();
    d_ptr->columnNameList.clear();
    d_ptr->IDList.clear();
}

int ONTable::newRow()
{
    if (d_ptr->columnList.size() == 0)
        return 0;

    // Assuming increasing ID of row in the list
    int availableID = d_ptr->IDList.size() > 0 ?
                      d_ptr->IDList.back() + 1 :
                      1;
    for (size_t i=0; i<d_ptr->columnList.size(); i++)
        d_ptr->columnList[i]->set(availableID, nullptr);
    d_ptr->IDList.push_back(availableID);
    return availableID;
}

int ONTable::newColumn(const std::string& name, ColumnType columnType)
{
    // Assuming increasing ID of column in the list
    int availableID = d_ptr->columnList.size() > 0 ?
                      d_ptr->columnList.back()->ID + 1 :
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
        case IntegerList:
            emptyColumn = new ONTableIntListColumn;
            break;
        default:
            return 0;
    }
    emptyColumn->ID = availableID;
    emptyColumn->typeID = columnType;
    d_ptr->columnList.push_back(emptyColumn);
    d_ptr->columnIDList.push_back(availableID);
    d_ptr->columnTypeIDList.push_back(columnType);
    d_ptr->columnNameList.push_back(name);

    return availableID;
}

ONTable::ColumnType ONTable::columnType(int columnID) const
{
    int columnIndex = d_ptr->getColumnIndexByID(columnID);
    if (columnIndex >= 0)
        return ColumnType(d_ptr->columnTypeIDList[columnIndex]);
    else
        return ColumnType::None;
}

ONTableDefaultValue ONTable::defaultValues() const
{
    return ONTableDefaultValue{ d_ptr->defaultIntValue,
                                d_ptr->defaultDoubleValue,
                                d_ptr->defaultStringValue };
}

void ONTable::setDefaultValues(const ONTableDefaultValue& values)
{
    d_ptr->defaultIntValue = values.intValue;
    d_ptr->defaultDoubleValue = values.doubleValue;
    d_ptr->defaultStringValue = values.stringValue;
}

int ONTable::readInt(int ID, int columnID) const
{
    int columnIndex = d_ptr->getColumnIndexByID(columnID);
#ifdef ONTABLE_COLUMN_TYPE_CHECK
    if (d_ptr->columnList[columnIndex]->typeID() != ColumnType::Integer)
        return defaultIntValue;
#endif
    ONTableIntColumn* column =
            dynamic_cast<ONTableIntColumn*>(d_ptr->columnList[columnIndex]);
    if (column && column->exists(ID))
        return column->valueAsInt(ID);
    else
        return d_ptr->defaultIntValue;
}

double ONTable::readDouble(int ID, int columnID) const
{
    int columnIndex = d_ptr->getColumnIndexByID(columnID);
#ifdef ONTABLE_COLUMN_TYPE_CHECK
    if (d_ptr->columnList[columnIndex]->typeID() != ColumnType::Double)
        return defaultDoubleValue;
#endif
    ONTableDoubleColumn* column =
            dynamic_cast<ONTableDoubleColumn*>(d_ptr->columnList[columnIndex]);
    if (column && column->exists(ID))
        return column->valueAsDouble(ID);
    else
        return d_ptr->defaultDoubleValue;
}

std::string ONTable::readString(int ID, int columnID) const
{
    int columnIndex = d_ptr->getColumnIndexByID(columnID);
#ifdef ONTABLE_COLUMN_TYPE_CHECK
    if (d_ptr->columnList[columnIndex]->typeID() != ColumnType::String)
        return defaultStringValue;
#endif
    ONTableStringColumn* column =
            dynamic_cast<ONTableStringColumn*>(d_ptr->columnList[columnIndex]);
    if (column && column->exists(ID))
        return column->valueAsString(ID);
    else
        return d_ptr->defaultStringValue;
}

std::vector<int> ONTable::readIntList(int ID, int columnID) const
{
    std::vector<int> valueList;
    int columnIndex = d_ptr->getColumnIndexByID(columnID);
#ifdef ONTABLE_COLUMN_TYPE_CHECK
    if (d_ptr->columnList[columnIndex]->typeID() != ColumnType::IntegerList)
        return std::vector<int>();
#endif

    ONTableIntListColumn* column =
            dynamic_cast<ONTableIntListColumn*>(d_ptr->columnList[columnIndex]);
    if (!(column && column->exists(ID)))
        return valueList;

    int valueCount;
    int* values = column->valueAsIntList(ID, valueCount);
    if (valueCount > 0)
        valueList = std::vector<int>(values, values + valueCount);
    delete[] values;
    return valueList;
}

bool ONTable::modify(int ID, int columnID, const int& value)
{
    int columnIndex = d_ptr->getColumnIndexByID(columnID);
#ifdef ONTABLE_COLUMN_TYPE_CHECK
    if (d_ptr->columnList[columnIndex]->typeID() != ColumnType::Integer)
        return false;
#endif
    dynamic_cast<ONTableIntColumn*>
            (d_ptr->columnList[columnIndex])->set(ID, value);
    return true;
}

bool ONTable::modify(int ID, int columnID, const double& value)
{
    int columnIndex = d_ptr->getColumnIndexByID(columnID);
#ifdef ONTABLE_COLUMN_TYPE_CHECK
    if (d_ptr->columnList[columnIndex]->typeID() != ColumnType::Double)
        return false;
#endif
    dynamic_cast<ONTableDoubleColumn*>
            (d_ptr->columnList[columnIndex])->set(ID, value);
    return true;
}

bool ONTable::modify(int ID, int columnID, const std::string& value)
{
    int columnIndex = d_ptr->getColumnIndexByID(columnID);
#ifdef ONTABLE_COLUMN_TYPE_CHECK
    if (d_ptr->columnList[columnIndex]->typeID() != ColumnType::String)
        return false;
#endif
    dynamic_cast<ONTableStringColumn*>
            (d_ptr->columnList[columnIndex])->set(ID, value);
    return true;
}

bool ONTable::modify(int ID, int columnID, const std::vector<int>& valueList)
{
    int columnIndex = d_ptr->getColumnIndexByID(columnID);
#ifdef ONTABLE_COLUMN_TYPE_CHECK
    if (d_ptr->columnList[columnIndex]->typeID() != ColumnType::IntegerList)
        return false;
#endif
    dynamic_cast<ONTableIntListColumn*>
            (d_ptr->columnList[columnIndex])->set(ID,
                                                  valueList.data(),
                                                  valueList.size());
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
       d_ptr->columnList[i]->remove(ID);
}

void ONTable::removeColumn(int columnID)
{
    std::vector<ONTableColumn*>::iterator i;
    for (i=d_ptr->columnList.begin(); i!=d_ptr->columnList.end(); i++)
        if ((*i)->ID == columnID)
            break;
    if (i!=d_ptr->columnList.end())
    {
        delete *i;
        d_ptr->columnList.erase(i);

        long index = i - d_ptr->columnList.begin();
        d_ptr->columnIDList.erase(d_ptr->columnIDList.begin() + index);
        d_ptr->columnTypeIDList.erase(d_ptr->columnTypeIDList.begin() + index);
        d_ptr->columnNameList.erase(d_ptr->columnNameList.begin() + index);
    }
}

std::string ONTable::bindingDirectory() const
{
    return d_ptr->bindingDirectory;
}

bool ONTable::setBindingDirectory(const std::string& path)
{
    if (!utils_isDirectory(path.c_str()))
        return false;
    d_ptr->bindingDirectory = path;
    return true;
}

std::string ONTable::fileSuffix() const
{
    return d_ptr->fileSuffix;
}

void ONTable::setFileSuffix(const std::string& suffix)
{
    d_ptr->fileSuffix = suffix;
}

bool ONTable::load()
{
    if (d_ptr->bindingDirectory.empty())
        return false;

    // Read column information from the index file
    std::string indexFilename = d_ptr->getIndexFilename();
    if (!utils_isFile(indexFilename.c_str()))
        return false;

    char buffer[ONTABLE_TABLE_IO_BUFFER_MAXLEN];
    FILE* f = fopen(indexFilename.c_str(), "rb");

    // Check for the index file header
    fread(buffer, 1, strlen(ONTABLE_TABLE_INDEX_FILE_HEADER), f);
    if (strncmp(buffer, ONTABLE_TABLE_INDEX_FILE_HEADER,
                strlen(ONTABLE_TABLE_INDEX_FILE_HEADER)) != 0)
    {
        fclose(f);
        return false;
    }
    // Ignore version check
    fseek(f, strlen(ONTABLE_TABLE_INDEX_FILE_VERSION), SEEK_CUR);

    clear();

    char* pos, *posS, *posE;
    int columnID, columnTypeID;
    std::string columnName;
    std::string columnFilename;
    while (!feof(f))
    {
        if (fgets(buffer, ONTABLE_TABLE_IO_BUFFER_MAXLEN - 1, f) != buffer)
            break;

        // Parse the column index, the column type ID and
        // the column name from each line
        columnID = static_cast<int>(strtol(buffer, &pos, 10));
        if (buffer == pos)
            continue;

        columnTypeID = static_cast<int>(strtol(pos + 1, &pos, 10));
        if (buffer == pos)
            continue;

        posS = strstr(pos + 1, "\"");
        if (!posS)
            continue;
        posE = strstr(posS + 1, "\"");
        if (!posE)
            continue;
        columnName = std::string(posS + 1, posE - posS - 1);

        if (!d_ptr->loadColumn(columnID, columnTypeID, columnName))
        {
            fclose(f);
            return false;
        }
    }
    fclose(f);
    return true;
}

bool ONTable::save()
{
    size_t i;
    for (i=0; i<d_ptr->columnList.size(); i++)
    {
        if (!d_ptr->saveColumn(i))
            return false;
    }

    // Update the index file
    FILE* f = fopen(d_ptr->getIndexFilename().c_str(), "wb");
    if (!f)
        return false;
    fputs(ONTABLE_TABLE_INDEX_FILE_HEADER, f);
    fputs(ONTABLE_TABLE_INDEX_FILE_VERSION, f);

    for (i=0; i<d_ptr->columnList.size(); i++)
    {
        fprintf(f, "%d,%d,\"%s\"\n",
                d_ptr->columnIDList[i],
                d_ptr->columnTypeIDList[i],
                d_ptr->columnNameList[i].c_str());
    }

    fclose(f);
    return true;
}

ONTablePrivate::ONTablePrivate()
{
    defaultIntValue = 0;
    defaultDoubleValue = 0.0;
}

ONTablePrivate::~ONTablePrivate()
{
    for (size_t i=0; i<columnList.size(); i++)
        delete columnList[i];
}

ONTablePrivate::ONTablePrivate(const ONTablePrivate& src)
{
    bindingDirectory = src.bindingDirectory;
    fileSuffix = src.fileSuffix;

    if (src.columnList.size() > 0)
    {
        // Do a deep copy for the columns
        ONTableColumn* newColumn;
        for (size_t i=0; i<src.columnList.size(); i++)
        {
            switch (src.columnList[i]->typeID)
            {
                case ONTable::ColumnType::Integer:
                    newColumn = new ONTableIntColumn(
                                    *(ONTableIntColumn*)src.columnList[i]);
                    break;
                case ONTable::ColumnType::Double:
                    newColumn = new ONTableDoubleColumn(
                                    *(ONTableDoubleColumn*)src.columnList[i]);
                    break;
                case ONTable::ColumnType::String:
                    newColumn = new ONTableStringColumn(
                                    *(ONTableStringColumn*)src.columnList[i]);
                    break;
                case ONTable::ColumnType::IntegerList:
                    newColumn = new ONTableIntListColumn(
                                    *(ONTableIntListColumn*)src.columnList[i]);
                    break;
                default:
                    newColumn = nullptr;
            }
            if (newColumn != nullptr)
                columnList.push_back(newColumn);
        }
        columnIDList = src.columnIDList;
        columnTypeIDList = src.columnTypeIDList;
        columnNameList = src.columnNameList;
        IDList = src.IDList;

        defaultIntValue = src.defaultIntValue;
        defaultDoubleValue = src.defaultDoubleValue;
    }
}

int ONTablePrivate::getColumnIndexByID(int columnID) const
{
    for (size_t i=0; i<columnIDList.size(); i++)
    {
        if (columnIDList[i] == columnID)
            return int(i);
    }
    return -1;
}

std::string ONTablePrivate::getIndexFilename() const
{
    std::string indexFilename;

    if (bindingDirectory.empty() ||
        !utils_isDirectory(bindingDirectory.c_str()))
        return indexFilename;
    indexFilename.append(bindingDirectory)
                 .append("/")
                 .append(ONTABLE_TABLE_INDEX_FILE_NAME);
    return indexFilename;
}

std::string ONTablePrivate::getColumnFilename(int columnID) const
{
    std::string filename;
    if (bindingDirectory.empty() ||
        !utils_isDirectory(bindingDirectory.c_str()))
        return filename;

    char ID[ONTABLE_TABLE_IO_BUFFER_MAXLEN];
    sprintf(ID, "%d", columnID);
    filename.append(bindingDirectory)
            .append("/")
            .append(ID)
            .append(fileSuffix);
    return filename;
}


bool ONTablePrivate::loadColumn(int columnID, int columnType,
                                const std::string& columnName)
{
    ONTableColumn* newColumn;
    switch (ONTable::ColumnType(columnType))
    {
        case ONTable::ColumnType::Integer:
            newColumn = new ONTableIntColumn;
            break;
        case ONTable::ColumnType::Double:
            newColumn = new ONTableDoubleColumn;
            break;
        case ONTable::ColumnType::String:
            newColumn = new ONTableStringColumn;
            break;
        case ONTable::ColumnType::IntegerList:
            newColumn = new ONTableIntListColumn;
            break;
        default:
            return false;
    }
    if (!(newColumn->setBindingFile(getColumnFilename(columnID).c_str()) &&
          newColumn->load()))
        return false;

    newColumn->ID = columnID;
    newColumn->typeID = columnType;
    columnList.push_back(newColumn);
    columnIDList.push_back(newColumn->ID);
    columnTypeIDList.push_back(newColumn->typeID);
    columnNameList.push_back(columnName);

    // Use the IDs of the first column as the record IDs
    if (IDList.size() == 0)
    {
        int* keyList = newColumn->keys();
        for (int j=0; j<newColumn->length(); j++)
            IDList.push_back(keyList[j]);
        delete[] keyList;
    }
    return true;
}

bool ONTablePrivate::saveColumn(int columnIndex)
{
    // TODO: save bindingFilename for each columns before saving
    ONTableColumn* column = columnList[columnIndex];
    std::string oldFilename;
    std::stringstream filename;
    oldFilename = column->bindingFile() == nullptr ?
                  "" :
                  column->bindingFile();
    filename.str("");
    filename << bindingDirectory
             << "/"
             << column->ID
             << fileSuffix;
    column->setBindingFile(filename.str().c_str());
    if (column->save())
        return true;
    else
    {
        column->setBindingFile(oldFilename.c_str());
        return false;
    }
}
