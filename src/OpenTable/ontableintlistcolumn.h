#ifndef ONTABLEINTLISTCOLUMN_H
#define ONTABLEINTLISTCOLUMN_H

#include <string>
#include "ontablecolumn.h"

#define ONTABLE_COLUMN_TYPE_INTLIST   8 & 1


class ONTableIntListColumn : public ONTableColumn
{
public:
    ONTableIntListColumn();
    ONTableIntListColumn(const ONTableIntListColumn& src);
    ~ONTableIntListColumn();

    virtual int* valueAsIntList(int key, int& count) const;
    virtual void set(int key, const int* values, int count);
    virtual void remove(int key);

    virtual bool load();
    virtual bool save();

private:
    ONTableColumnPrivate* d;
};

#endif // ONTABLEINTLISTCOLUMN_H
