#ifndef TABLEMODEL_P_H
#define TABLEMODEL_P_H

#include "OpenTable/ontable_p.h"


class TableModelPrivate : public ONTablePrivate
{
public:
    int getRowID(int rowIndex) const;
};

#endif // TABLEMODEL_P_H
