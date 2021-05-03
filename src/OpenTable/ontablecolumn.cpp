#include <cstdio>
#include <cstring>
#include "ontablecolumn.h"
#include "ontablecolumn_p.h"


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

ONTableColumnPrivate::ONTableColumnPrivate()
{
    bindingFile = nullptr;
}

ONTableColumnPrivate::ONTableColumnPrivate(const ONTableColumnPrivate& src)
{
    if (src.bindingFile == nullptr)
        bindingFile = nullptr;
    else
    {
        bindingFile = new char[strlen(src.bindingFile) + 1];
        strcpy(bindingFile, src.bindingFile);
    }

    // Do a shallow copy for the data (pointers), as the data type is unknown
    data = src.data;
}
