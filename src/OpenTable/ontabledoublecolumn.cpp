#include <cstring>
#include "ontabledoublecolumn.h"
#include "ontablecolumn_p.h"

#define ONTABLE_COLUMN_DOUBLE_BUFFER_LEN 256


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

double ONTableDoubleColumn::valueAsDouble(int key) const
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

bool ONTableDoubleColumn::load()
{
    if (!d_ptr->bindingFile)
        return false;

    int key;
    double value;
    char* data;
    char* pos, *pos2;
    char buffer[ONTABLE_COLUMN_DOUBLE_BUFFER_LEN];
    FILE* f = fopen(d_ptr->bindingFile, "rb");
    while (true)
    {
        if (fgets(buffer, ONTABLE_COLUMN_DOUBLE_BUFFER_LEN, f) != buffer)
            break;

        // Try to parse the record ID and the record value
        pos = strstr(buffer, d_ptr->fieldDelimiter);
        if (!pos)
            continue;
        key = static_cast<int>(strtold(buffer, &pos2));
        if (key < 0 || buffer == pos2)
            continue;
        value = double(strtold(pos + 1, &pos2));
        if (pos + 1 == pos2)
            continue;

        data = reinterpret_cast<char*>(new double);
        memcpy(data, &value, sizeof(double));
        d_ptr->data.insert(std::make_pair(key, data));

        if (feof(f))
            break;
    }
    fclose(f);
    return true;
}

bool ONTableDoubleColumn::save()
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
        memcpy(&value, i->second, sizeof(double));
        fprintf(f, "%d%s%f%s",
                i->first,
                d->fieldDelimiter,
                value,
                d->recordDelimiter);
    }
    fclose(f);
    return true;
}
