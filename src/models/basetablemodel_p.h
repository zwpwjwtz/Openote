#ifndef BASETABLEMODEL_P_H
#define BASETABLEMODEL_P_H

#include "OpenTable/ontable_p.h"


class BaseBookModel;

class BaseTableModelPrivate : public ONTablePrivate
{
public:
    BaseTableModelPrivate();
    BaseTableModelPrivate(const BaseTableModelPrivate& src);
    ~BaseTableModelPrivate();

    BaseBookModel* book;
    std::vector<int> columnReferenceIDList;
};

#endif // BASETABLEMODEL_P_H
