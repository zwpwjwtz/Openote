#include <cstdlib>
#include <cstring>
#include "ontableintcolumn.h"
#include "ontablecolumn_p.h"

#define ONTABLE_COLUMN_INT_BUFFER_LEN    64


ONTableIntColumn::ONTableIntColumn()
{
    d = d_ptr;
    typeID = ONTABLE_COLUMN_TYPE_INT;
}

ONTableIntColumn::ONTableIntColumn(const ONTableIntColumn& src) :
    ONTableColumn (src)
{
    d = d_ptr;
    d->data.clear();

    // Do a deep copy for the data
    int* newData;
    std::map<int, char*>::const_iterator i;
    for (i=src.d->data.cbegin(); i!=src.d->data.cend(); i++)
    {
        if (i->second == nullptr)
        {
            d->data.insert(std::make_pair(i->first, nullptr));
            continue;
        }
        newData = new int;
        memcpy(newData, i->second, sizeof(int));
        d->data.insert(std::make_pair(i->first,
                                      reinterpret_cast<char*>(newData)));
    }
}

ONTableIntColumn::~ONTableIntColumn()
{
    std::map<int, char*>::iterator i;
    for (i=d->data.begin(); i!=d->data.end(); i++)
        delete (*i).second;
}

int ONTableIntColumn::valueAsInt(int key) const
{
    int intValue = 0;
    char* data = d->data.at(key);
    if (data)
        memcpy(&intValue, data, sizeof(int));
    return intValue;
}

void ONTableIntColumn::set(int key, int value)
{
    std::map<int, char*>::iterator pos = d->data.find(key);
    if (pos == d->data.cend())
    {
        // The specified key does not exist
        void* data = new int;
        memcpy(data, &value, sizeof(int));
        d->data.insert(std::make_pair(key, static_cast<char*>(data)));
    }
    else
    {
        if ((*pos).second == nullptr)
            (*pos).second = reinterpret_cast<char*>(new int);
        memcpy((*pos).second, &value, sizeof(int));
    }
}

void ONTableIntColumn::duplicate(int oldKey, int newKey)
{
    std::map<int, char*>::iterator pos1 = d_ptr->data.find(oldKey);
    std::map<int, char*>::iterator pos2 = d_ptr->data.find(newKey);
    if (pos1 == d_ptr->data.end())
        return;

    if (pos2 == d_ptr->data.end())
    {
        int* data = nullptr;
        if ((*pos1).second != nullptr)
        {
            data = new int;
            memcpy(data, (*pos1).second, sizeof(int));
        }
        d_ptr->data.insert(pos2,
                       std::make_pair(newKey, reinterpret_cast<char*>(data)));
    }
    else
    {
        if ((*pos1).second == nullptr)
        {
            if ((*pos2).second != nullptr)
                delete (*pos2).second;
            (*pos2).second = nullptr;
        }
        else
        {
            if ((*pos2).second == nullptr)
                (*pos2).second = reinterpret_cast<char*>(new int);
            memcpy((*pos2).second, (*pos1).second, sizeof(int));
        }
    }
}

void ONTableIntColumn::remove(int key)
{
    std::map<int, char*>::const_iterator pos = d->data.find(key);
    if (pos != d->data.cend())
    {
        delete (*pos).second;
        d->data.erase(key);
    }
}

bool ONTableIntColumn::load()
{
    if (!d_ptr->bindingFile)
        return false;

    int key, value;
    char* data;
    char* pos, *pos2;
    char buffer[ONTABLE_COLUMN_INT_BUFFER_LEN];
    FILE* f = fopen(d_ptr->bindingFile, "rb");
    if (!f)
        return false;

    while (true)
    {
        if (fgets(buffer, ONTABLE_COLUMN_INT_BUFFER_LEN, f) != buffer)
            break;

        // Try to parse the record ID and the record value
        pos = strstr(buffer, d_ptr->fieldDelimiter);
        if (!pos)
            continue;
        key = static_cast<int>(strtol(buffer, &pos2, 10));
        if (key < 0 || buffer == pos2)
            continue;
        value = int(strtold(pos + 1, &pos2));
        if (pos + 1 == pos2)
            continue;

        data = reinterpret_cast<char*>(new int);
        memcpy(data, &value, sizeof(int));
        d_ptr->data.insert(std::make_pair(key, data));

        if (feof(f))
            break;
    }
    fclose(f);
    return true;
}

bool ONTableIntColumn::save()
{
    if (!d_ptr->bindingFile)
        return false;

    int value;
    FILE* f = fopen(d_ptr->bindingFile, "wb");
    if (!f)
        return false;

    std::map<int, char*>::const_iterator i;
    for (i=d_ptr->data.cbegin(); i!=d_ptr->data.cend(); i++)
    {
        if (i->second == nullptr)
            continue;
        memcpy(&value, i->second, sizeof(int));
        fprintf(f, "%d%s%d%s",
                i->first,
                d->fieldDelimiter,
                value,
                d->recordDelimiter);
    }
    fclose(f);
    return true;
}
