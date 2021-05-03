#include <cstdio>
#include <cstring>
#include "ontablecolumn.h"
#include "ontablecolumn_p.h"

#define ONTABLE_COLUMN_IO_DELIMITER_RECORD    "\n"
#define ONTABLE_COLUMN_IO_DELIMITER_FIELD     ","


ONTableColumn::ONTableColumn()
{
    ID = 0;
    typeID = ONTABLE_COLUMN_TYPE_BASE;
    d_ptr = new ONTableColumnPrivate();
}

ONTableColumn::ONTableColumn(const ONTableColumn& src)
{
    ID = src.ID;
    typeID = src.typeID;
    d_ptr = new ONTableColumnPrivate(*(src.d_ptr));
}

ONTableColumn::ONTableColumn(ONTableColumnPrivate* data)
{
    if (data == nullptr)
        ONTableColumn();
    else
        d_ptr = data;
}

ONTableColumn::~ONTableColumn()
{
    if (d_ptr->bindingFile != nullptr)
        delete[] d_ptr->bindingFile;
    delete[] d_ptr->recordDelimiter;
    delete[] d_ptr->fieldDelimiter;

    delete d_ptr;
}

void ONTableColumn::clear() const
{
    d_ptr->data.clear();
}

int ONTableColumn::length() const
{
    return d_ptr->data.size();
}

bool ONTableColumn::exists(int key) const
{
    return d_ptr->data.find(key) != d_ptr->data.cend();
}

char* ONTableColumn::value(int key) const
{
    return d_ptr->data.at(key);
}

int* ONTableColumn::keys() const
{
    int* keyList = new int[d_ptr->data.size()];
    int i = 0;
    std::map<int, char*>::const_iterator j;
    for (j=d_ptr->data.cbegin(); j!=d_ptr->data.cend(); j++)
    {
        keyList[i] = (*j).first;
        i++;
    }
    return keyList;
}

char** ONTableColumn::values() const
{
    char** valueList = new char*[d_ptr->data.size()];
    int i = 0;
    std::map<int, char*>::const_iterator j;
    for (j=d_ptr->data.cbegin(); j!=d_ptr->data.cend(); j++)
    {
        valueList[i] = (*j).second;
        i++;
    }
    return valueList;
}

void ONTableColumn::set(int key, char* value)
{
    std::map<int, char*>::iterator pos = d_ptr->data.find(key);
    if (pos != d_ptr->data.end())
        (*pos).second = value;
    else
        d_ptr->data.insert(std::make_pair(key, value));
}

void ONTableColumn::remove(int key)
{
    d_ptr->data.erase(key);
}

const char* ONTableColumn::bindingFile()
{
    return d_ptr->bindingFile;
}

bool ONTableColumn::setBindingFile(const char* filename)
{
    if (filename == nullptr)
        return false;

    delete[] d_ptr->bindingFile;
    d_ptr->bindingFile = new char[strlen(filename) + 1];
    strcpy(d_ptr->bindingFile, filename);
    return true;
}

const char* ONTableColumn::recordDelimiter() const
{
    return d_ptr->recordDelimiter;
}

void ONTableColumn::setRecordDelimiter(const char* delimiter)
{
    delete d_ptr->recordDelimiter;

    if (delimiter == nullptr)
        delimiter = ONTABLE_COLUMN_IO_DELIMITER_RECORD;
    d_ptr->recordDelimiter = new char[strlen(delimiter) + 1];
    strcpy(d_ptr->recordDelimiter, delimiter);
}

const char* ONTableColumn::fieldDelimiter() const
{
    return d_ptr->fieldDelimiter;
}

void ONTableColumn::setFieldDelimiter(const char* delimiter)
{
    delete d_ptr->fieldDelimiter;

    if (delimiter == nullptr)
        delimiter = ONTABLE_COLUMN_IO_DELIMITER_FIELD;
    d_ptr->fieldDelimiter = new char[strlen(delimiter) + 1];
    strcpy(d_ptr->fieldDelimiter, delimiter);
}

ONTableColumnPrivate::ONTableColumnPrivate()
{
    bindingFile = nullptr;

    recordDelimiter = new char[strlen(ONTABLE_COLUMN_IO_DELIMITER_RECORD) + 1];
    strcpy(recordDelimiter, ONTABLE_COLUMN_IO_DELIMITER_RECORD);
    fieldDelimiter = new char[strlen(ONTABLE_COLUMN_IO_DELIMITER_FIELD) + 1];
    strcpy(fieldDelimiter, ONTABLE_COLUMN_IO_DELIMITER_FIELD);
}

ONTableColumnPrivate::ONTableColumnPrivate(const ONTableColumnPrivate& src)
{
    // Do a deep copy for the char arrays
    if (src.bindingFile == nullptr)
        bindingFile = nullptr;
    else
    {
        bindingFile = new char[strlen(src.bindingFile) + 1];
        strcpy(bindingFile, src.bindingFile);
    }

    recordDelimiter = new char[strlen(src.recordDelimiter) + 1];
    strcpy(recordDelimiter, src.recordDelimiter);
    fieldDelimiter = new char[strlen(src.fieldDelimiter) + 1];
    strcpy(fieldDelimiter, src.fieldDelimiter);

    // Do a shallow copy for the data (pointers), as the data type is unknown
    data = src.data;
}
