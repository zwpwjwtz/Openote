#include <cstring>
#include "ontableintcolumn.h"
#include "ontablecolumn_p.h"

#define ONTABLE_COLUMN_INT_BUFFER_LEN    64


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

bool ONTableIntColumn::load()
{
    if (!d_ptr->bindingFile)
        return false;

    int key, value;
    char* data;
    char* pos, *pos2;
    char buffer[ONTABLE_COLUMN_INT_BUFFER_LEN];
    FILE* f = fopen(d_ptr->bindingFile, "rb");
    while (true)
    {
        if (fgets(buffer, ONTABLE_COLUMN_INT_BUFFER_LEN, f) != buffer)
            break;

        // Try to parse the record ID and the record value
        pos = strstr(buffer, d_ptr->fieldDelimiter);
        if (!pos)
            continue;
        key = static_cast<int>(strtold(buffer, &pos2));
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
