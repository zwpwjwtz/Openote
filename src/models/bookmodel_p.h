#ifndef BOOKMODEL_P_H
#define BOOKMODEL_P_H

#include "basebookmodel.h"


class TableModel;
class BookModel;

class BookModelPrivate : public BaseBookModel
{
public:
    BookModelPrivate(BookModel* parent);
    BaseTableModel* newBaseTable(const BaseTableModel* src);

protected:
    BookModel* q_ptr;
};

#endif // BOOKMODEL_P_H
