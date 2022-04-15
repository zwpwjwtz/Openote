#ifndef TABLEMODEL_P_H
#define TABLEMODEL_P_H

#include "basetablemodel_p.h"


class TableModelPrivate : public BaseTableModelPrivate
{
public:
    TableModelPrivate() {}
    TableModelPrivate(const TableModelPrivate& src);
    ~TableModelPrivate();
};

#endif // TABLEMODEL_P_H
