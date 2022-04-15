#ifndef BASETABLEMODEL_H
#define BASETABLEMODEL_H

#include "OpenTable/ontable.h"


class BaseBookModel;
class BaseTableModelPrivate;

class BaseTableModel : public ONTable
{
public:
    BaseTableModel();
    BaseTableModel(const BaseTableModel& src);

    int rowID(int row) const;
    int columnID(int column) const;

    bool bindBookModel(BaseBookModel *model);
    bool setColumnReference(int columnIndex, int referenceID);

    virtual bool duplicateRow(int row);
    virtual bool duplicateColumn(int column, const std::string& newName);

    void removeColumn(int columnID);

    bool load();

protected:
    BaseTableModelPrivate* d;
    BaseTableModel(BaseTableModelPrivate* data);
};

#endif // BASETABLEMODEL_H
