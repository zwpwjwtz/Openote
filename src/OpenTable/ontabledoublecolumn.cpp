#include <cstring>
#include "ontabledoublecolumn.h"
#include "ontablecolumn_p.h"


ONTableDoubleColumn::ONTableDoubleColumn()
{
    d = d_ptr;
    ID = 0;
    typeID = ONTABLE_COLUMN_TYPE_DOUBLE;
}

ONTableDoubleColumn::~ONTableDoubleColumn()
{
    std::map<int, char*>::iterator i;
    for (i=d->data.begin(); i!=d->data.end(); i++)
        delete (*i).second;
}

double ONTableDoubleColumn::valueAsInt(int key) const
{
    double doubleValue = 0.0;
    char* data = d->data.at(key);
    if (data)
        memcpy(&doubleValue, data, sizeof(double));
    return doubleValue;
}

void ONTableDoubleColumn::set(int key, double value)
{
    std::map<int, char*>::iterator pos = d->data.find(key);
    if (pos == d->data.cend())
    {
        // The specified key does not exist
        void* data = new double;
        memcpy(data, &value, sizeof(double));
        d->data.insert(std::make_pair(key, static_cast<char*>(data)));
    }
    else
    {
        if ((*pos).second == nullptr)
            (*pos).second = reinterpret_cast<char*>(new double);
        memcpy((*pos).second, &value, sizeof(double));
    }
}

void ONTableDoubleColumn::remove(int key)
{
    std::map<int, char*>::const_iterator pos = d->data.find(key);
    if (pos != d->data.cend())
    {
        delete (*pos).second;
        d->data.erase(key);
    }
}
