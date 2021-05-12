#include <climits>
#include <cstring>
#include "ontablestringcolumn.h"
#include "ontablecolumn_p.h"

#define ONTABLE_COLUMN_STRING_BUFFER_LEN 2048


ONTableStringColumn::ONTableStringColumn()
{
    d = d_ptr;
    ID = 0;
    typeID = ONTABLE_COLUMN_TYPE_STRING;
}

ONTableStringColumn::ONTableStringColumn(const ONTableStringColumn& src) :
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
        newData = new char[dataLength + sizeof(int)];
        memcpy(newData, i->second, dataLength + sizeof(int));
        d->data.insert(std::make_pair(i->first, newData));
    }
}

ONTableStringColumn::~ONTableStringColumn()
{
    std::map<int, char*>::iterator i;
    for (i=d->data.begin(); i!=d->data.end(); i++)
        delete[] (*i).second;
}

std::string ONTableStringColumn::valueAsString(int key) const
{
    char* data = d->data.at(key);
    if (data)
    {
        int dataLength;
        memcpy(&dataLength, data, sizeof(int));
        return std::string(data + sizeof(int), dataLength);
    }
    return std::string();
}

void ONTableStringColumn::set(int key, const std::string& value)
{
    // Construct an char array to store the content of the string
    // The first sizeof(int) bytes is used to store the length of the string
    size_t stringLength = value.length();
    if (stringLength + 4 > INT_MAX)
        return;

    char* data = new char[sizeof(int) + stringLength];
    memcpy(data, &stringLength, sizeof(int));
    memcpy(data + sizeof(int), value.c_str(), stringLength);

    std::map<int, char*>::iterator pos = d->data.find(key);
    if (pos == d->data.cend())
        d->data.insert(std::make_pair(key, data));
    else
    {
        if ((*pos).second != nullptr)
            delete (*pos).second;
        (*pos).second = data;
    }
}

void ONTableStringColumn::duplicate(int oldKey, int newKey)
{
    std::map<int, char*>::iterator pos1 = d_ptr->data.find(oldKey);
    std::map<int, char*>::iterator pos2 = d_ptr->data.find(newKey);
    if (pos1 == d_ptr->data.end())
        return;

    char* data = nullptr;
    size_t stringLength;
    if ((*pos1).second != nullptr)
    {
        memcpy(&stringLength, (*pos1).second, sizeof(int));
        data = new char[sizeof(int) + stringLength];
        memcpy(data, (*pos1).second, sizeof(int) + stringLength);
    }
    if ((pos2 == d_ptr->data.end()))
        d_ptr->data.insert(pos2, std::make_pair(newKey, data));
    else
    {
        if ((*pos2).second != nullptr)
            delete (*pos2).second;
        (*pos2).second = data;
    }
}

void ONTableStringColumn::remove(int key)
{
    std::map<int, char*>::const_iterator pos = d->data.find(key);
    if (pos != d->data.cend())
    {
        delete static_cast<char*>((*pos).second);
        d->data.erase(key);
    }
}

bool ONTableStringColumn::load()
{
    if (!d_ptr->bindingFile)
        return false;

    int key;
    int valueLength;
    char* data;
    char* pos, *pos2, *posE;
    char* buffer = new char[ONTABLE_COLUMN_STRING_BUFFER_LEN];
    FILE* f = fopen(d_ptr->bindingFile, "rb");
    if (!f)
        return false;

    while (true)
    {
        // TODO: use d_ptr->recordDelimiter to define the read boundary
        if (fgets(buffer, ONTABLE_COLUMN_STRING_BUFFER_LEN, f) != buffer)
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

        pos2 = posE + strlen(d->fieldDelimiter);
        data = new char[valueLength + 4];
        memcpy(data, &valueLength, sizeof(int));
        memcpy(data + 4, pos2, valueLength);
        d_ptr->data.insert(std::make_pair(key, data));

        if (feof(f))
            break;
    }
    fclose(f);
    return true;
}

bool ONTableStringColumn::save()
{
    if (!d_ptr->bindingFile)
        return false;

    FILE* f = fopen(d_ptr->bindingFile, "wb");
    if (!f)
        return false;

    int valueLength;
    std::map<int, char*>::const_iterator i;
    for (i=d_ptr->data.cbegin(); i!=d_ptr->data.cend(); i++)
    {
        if (i->second == nullptr)
            continue;
        memcpy(&valueLength, i->second, sizeof(int));
        fprintf(f, "%d%s", i->first, d->fieldDelimiter);
        fprintf(f, "%d%s", valueLength, d->fieldDelimiter);
        fwrite(i->second + sizeof(int), valueLength, 1, f);
        fputs(d->recordDelimiter, f);
    }
    fclose(f);
    return true;
}
