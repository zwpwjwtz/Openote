#ifndef BOOKMODEL_P_H
#define BOOKMODEL_P_H

#include <list>
#include <QMap>
#include "OpenTable/onbook_p.h"


class TableModel;
class BookModel;

class BookModelPrivate : public ONBookPrivate
{
public:
    BookModel* q_ptr;

    /* Note: Here "tableList" is to shadow intentionally
     *       the variable of the same name in the parent class.
     *       This is to solve the pointer-drifting issues
     *       caused by multiple inheritance, with "QObject" being
     *       the first parent class of "BookModel".
     *       As a result, all functions altering this variable
     *       must be rewritten to provide correct results.
     */
    std::vector<TableModel*> tableList;

    virtual bool loadTable(int tableID, const std::string& tableName);

    template <typename T>
    static void removeDuplicate(std::list<T>& valueList,
                                QMap<int, int>& indexMap);
};

#endif // BOOKMODEL_P_H
