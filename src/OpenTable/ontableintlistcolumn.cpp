#include <climits>
#include <cstdlib>
#include <cstring>
#include "ontableintlistcolumn.h"
#include "ontablecolumn_p.h"

#define ONTABLE_COLUMN_INTLIST_BUFFER_LEN 2048


ONTableIntListColumn::ONTableIntListColumn()
{
    d = d_ptr;
    ID = 0;
    typeID = ONTABLE_COLUMN_TYPE_INTLIST;
}

ONTableIntListColumn::ONTableIntListColumn(const ONTableIntListColumn& src) :
    ONTableColumn (src)
{
    d = d_ptr;
    d->data.clear();

    // Do a deep copy for the data
    char* newData;
    int dataLength;
    std::map<int, char*>::const_iterator i;
    for (i=src.d->data.cbegin(); i!=src.d->data.cend(); i++)
    {
        if (i->second == nullptr)
        {
            d->data.insert(std::make_pair(i->first, nullptr));
            continue;
        }
        memcpy(&dataLength, i->second, sizeof(int));
        if (dataLength < 0)
            continue;
        newData = new char[sizeof(int) * (1 + dataLength)];
        memcpy(newData, i->second, sizeof(int) * (1 + dataLength));
        d->data.insert(std::make_pair(i->first, newData));
    }
}

ONTableIntListColumn::~ONTableIntListColumn()
{
    std::map<int, char*>::iterator i;
    for (i=d->data.begin(); i!=d->data.end(); i++)
        delete[] (*i).second;
}

int* ONTableIntListColumn::valueAsIntList(int key, int& count) const
{
    count = 0;
    int* valueList = nullptr;
    char* data = d->data.at(key);
    if (data)
    {
        memcpy(&count, data, sizeof(int));
        if (count >= 0)
        {
            valueList = new int[count];
            if (!valueList)
                count = 0;
            else
                memcpy(valueList, data + sizeof(int), count * sizeof(int));
        }
    }
    return valueList;
}

void ONTableIntListColumn::set(int key, const int* valueList, int count)
{
    // Construct an int array to store the content of the string
    // The first sizeof(int) bytes is used to store the length of the array
    if (count < 0 || count + 4 > INT_MAX)
        return;

    char* data = new char[sizeof(int) * (1 + count)];
    memcpy(data, &count, sizeof(int));
    memcpy(data + sizeof(int), valueList, sizeof(int) * count);

    std::map<int, char*>::iterator pos = d->data.find(key);
    if (pos == d->data.cend())
        d->data.insert(std::make_pair(key, data));
    else
    {
        if ((*pos).second != nullptr)
            delete[] (*pos).second;
        (*pos).second = data;
    }
}

void ONTableIntListColumn::duplicate(int oldKey, int newKey)
{
    std::map<int, char*>::iterator pos1 = d_ptr->data.find(oldKey);
    std::map<int, char*>::iterator pos2 = d_ptr->data.find(newKey);
    if (pos1 == d_ptr->data.end())
        return;

    int count;
    int* data = nullptr;
    if ((*pos1).second != nullptr)
    {
        memcpy(&count, (*pos1).second, sizeof(int));
        data = new int[sizeof(int) * (1 + count)];
        memcpy(data, (*pos1).second, sizeof(int) * (1 + count));
    }
    if ((pos2 == d_ptr->data.end()))
        d_ptr->data.insert(pos2,
                       std::make_pair(newKey, reinterpret_cast<char*>(data)));
    else
    {
        if ((*pos2).second == nullptr)
            delete[] (*pos2).second;
        (*pos2).second = reinterpret_cast<char*>(data);
    }
}

void ONTableIntListColumn::remove(int key)
{
    std::map<int, char*>::const_iterator pos = d->data.find(key);
    if (pos != d->data.cend())
    {
        delete[] static_cast<char*>((*pos).second);
        d->data.erase(key);
    }
}

bool ONTableIntListColumn::load()
{
    if (!d_ptr->bindingFile)
        return false;

    int i, key;
    int valueLength;
    int* data;
    char* pos, *pos2, *posE;
    char* buffer = new char[ONTABLE_COLUMN_INTLIST_BUFFER_LEN];
    FILE* f = fopen(d_ptr->bindingFile, "rb");
    if (!f)
        return false;

    while (true)
    {
        // TODO: use d_ptr->recordDelimiter to define the read boundary
        if (fgets(buffer, ONTABLE_COLUMN_INTLIST_BUFFER_LEN, f) != buffer)
            break;

        // Try to parse the record ID
        pos = strstr(buffer, d_ptr->fieldDelimiter);
        if (!pos)
            continue;
        key = static_cast<int>(strtol(buffer, &posE, 10));
        if (key < 0 || buffer == posE)
            continue;

        // Try to parse the record length
        pos2 = posE + strlen(d->fieldDelimiter);
        pos = strstr(pos2, d_ptr->fieldDelimiter);
        if (!pos)
            continue;
        valueLength = static_cast<int>(strtol(pos2, &posE, 10));
        if (valueLength < 0 || pos2 == posE || posE > pos)
            continue;

        // Try to parse each value in the record
        pos2 = posE + strlen(d->fieldDelimiter);
        data = new int[1 + valueLength];
        memset(data, valueLength, sizeof(int));
        memcpy(data, &valueLength, sizeof(int));
        for (i=1; i<valueLength + 1; i++)
        {
            data[i] = int(strtol(pos2, &posE, 10));
            if (posE == pos2)
                break;
            pos = strstr(pos2, d_ptr->fieldDelimiter);
            if (!pos)
                break;
            pos2 = pos + strlen(d->fieldDelimiter);
        }
        d_ptr->data.insert(std::make_pair(key, reinterpret_cast<char*>(data)));

        if (feof(f))
            break;
    }

    fclose(f);
    delete[] buffer;
    return true;
}

bool ONTableIntListColumn::save()
{
    if (!d_ptr->bindingFile)
        return false;

    FILE* f = fopen(d_ptr->bindingFile, "wb");
    if (!f)
        return false;

    int* value;
    int j, valueLength;
    std::map<int, char*>::const_iterator i;
    for (i=d_ptr->data.cbegin(); i!=d_ptr->data.cend(); i++)
    {
        if (i->second == nullptr)
            continue;
        memcpy(&valueLength, i->second, sizeof(int));
        fprintf(f, "%d%s", i->first, d->fieldDelimiter);
        fprintf(f, "%d%s", valueLength, d->fieldDelimiter);
        for (j=0; j<valueLength; j++)
        {
            value = reinterpret_cast<int*>(i->second + sizeof(int)) + j;
            if (j == 0)
                fprintf(f, "%d", *value);
            else
                fprintf(f, "%s%d", d->fieldDelimiter, *value);
        }
        fputs(d->recordDelimiter, f);
    }
    fclose(f);
    return true;
}
