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

ONTableStringColumn::~ONTableStringColumn()
{
    std::map<int, char*>::iterator i;
    for (i=d->data.begin(); i!=d->data.end(); i++)
        delete static_cast<char*>((*i).second);
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
    unsigned int valueLength;
    char* data;
    char* pos, *pos2;
    char* buffer = new char[ONTABLE_COLUMN_STRING_BUFFER_LEN];
    FILE* f = fopen(d_ptr->bindingFile, "rb");
    while (true)
    {
        if (fgets(buffer, ONTABLE_COLUMN_STRING_BUFFER_LEN, f) != buffer)
            break;

        // Try to parse the record ID and the record value
        pos = strstr(buffer, d_ptr->recordDelimiter);
        if (!pos)
            continue;
        key = static_cast<int>(strtold(buffer, &pos2));
        if (key < 0 || buffer == pos2)
            continue;
        pos = strstr(pos2, d_ptr->recordDelimiter);
        if (pos + 1 == pos2)
            continue;

        valueLength = pos2 - pos;
        data = new char[valueLength + 4];
        memcpy(data, &valueLength, sizeof(int));
        memcpy(data + 4, pos2 + 1, valueLength);
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

    double value;
    std::map<int, char*>::const_iterator i;
    for (i=d_ptr->data.cbegin(); i!=d_ptr->data.cend(); i++)
    {
        memcpy(&value, i->second, sizeof(int));
        fprintf(f, "%d%s%f%s",
                i->first,
                d->fieldDelimiter,
                value,
                d->recordDelimiter);
    }
    fclose(f);
    return true;
}
