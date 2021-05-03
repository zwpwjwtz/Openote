#include <climits>
#include <cstring>
#include "ontablestringcolumn.h"
#include "ontablecolumn_p.h"


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
