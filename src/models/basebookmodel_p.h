#ifndef BASEBOOKMODEL_P_H
#define BASEBOOKMODEL_P_H

#include <list>
#include <map>
#include "basetablemodel.h"
#include "OpenTable/onbook_p.h"


class BaseBookModel;

class BaseBookModelPrivate : public ONBookPrivate
{
public:
    BaseBookModelPrivate(BaseBookModel* parent = nullptr);
    virtual bool loadTable(int tableID, const std::string& tableName);

    template <typename T>
    static void removeDuplicate(std::list<T>& valueList,
                                std::map<int, int>& indexMap);

protected:
    BaseBookModel* q;
};

#endif // BASEBOOKMODEL_P_H
