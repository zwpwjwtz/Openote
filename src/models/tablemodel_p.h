#ifndef TABLEMODEL_P_H
#define TABLEMODEL_P_H

#include <QChar>
#include "basetablemodel_p.h"


class TableModelPrivate : public BaseTableModelPrivate
{
public:
    TableModelPrivate() {}
    TableModelPrivate(const TableModelPrivate& src);
    ~TableModelPrivate();

    const QChar rowHeaderPlaceholder = ' ';
};

#endif // TABLEMODEL_P_H
