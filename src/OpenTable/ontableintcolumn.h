#ifndef ONTABLEINTCOLUMN_H
#define ONTABLEINTCOLUMN_H

#include "ontablecolumn.h"

#define ONTABLE_COLUMN_TYPE_INT    1


class ONTableIntColumn : public ONTableColumn
{
public:
    ONTableIntColumn();
    ~ONTableIntColumn();

    virtual int valueAsInt(int key) const;
    virtual void set(int key, int value = 0);
    virtual void remove(int key);

private:
    ONTableColumnPrivate* d;
};

#endif // ONTABLEINTCOLUMN_H
