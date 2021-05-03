#include <cstring>
#include "ontableintcolumn.h"
#include "ontablecolumn_p.h"


ONTableIntColumn::ONTableIntColumn()
{
    d = d_ptr;
    ID = 0;
    typeID = ONTABLE_COLUMN_TYPE_INT;
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

void ONTableIntColumn::remove(int key)
{
    std::map<int, char*>::const_iterator pos = d->data.find(key);
    if (pos != d->data.cend())
    {
        delete (*pos).second;
        d->data.erase(key);
    }
}
