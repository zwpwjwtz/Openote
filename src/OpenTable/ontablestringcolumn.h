#ifndef ONTABLESTRINGCOLUMN_H
#define ONTABLESTRINGCOLUMN_H

#include <string>
#include "ontablecolumn.h"

#define ONTABLE_COLUMN_TYPE_STRING    4


class ONTableStringColumn : public ONTableColumn
{
public:
    ONTableStringColumn();
    ~ONTableStringColumn();

    virtual std::string valueAsString(int key) const;
    virtual void set(int key, const std::string& value);
    virtual void remove(int key);

    virtual bool load();
    virtual bool save();

private:
    ONTableColumnPrivate* d;
};

#endif // ONTABLESTRINGCOLUMN_H
