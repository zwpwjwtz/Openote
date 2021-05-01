#ifndef ONTABLEDOUBLECOLUMN_H
#define ONTABLEDOUBLECOLUMN_H

#include "ontablecolumn.h"

#define ONTABLE_COLUMN_TYPE_DOUBLE    2


class ONTableDoubleColumn : public ONTableColumn
{
public:
    ONTableDoubleColumn();
    ~ONTableDoubleColumn();

    virtual double valueAsInt(int key) const;
    virtual void set(int key, double value);
    virtual void remove(int key);

private:
    ONTableColumnPrivate* d;
};

#endif // ONTABLEDOUBLECOLUMN_H
