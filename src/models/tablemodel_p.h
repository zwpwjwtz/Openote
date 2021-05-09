#ifndef TABLEMODEL_P_H
#define TABLEMODEL_P_H

#include <QList>
#include <QVariant>
#include "OpenTable/ontable_p.h"


class TableModelPrivate : public ONTablePrivate
{
public:
    TableModelPrivate() {}
    TableModelPrivate(const TableModelPrivate& src);

    QList<int> columnReferenceIDList;

    int getRowID(int rowIndex) const;
};

#endif // TABLEMODEL_P_H
