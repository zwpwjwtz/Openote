#ifndef BASEBOOKMODEL_H
#define BASEBOOKMODEL_H

#include "OpenTable/onbook.h"
#include "basetablemodel.h"
#include "bookindex.h"


class BaseBookModelPrivate;

class BaseBookModel : public ONBook
{
public:
    BaseBookModel();

    bool load();

    int tableCount() const;
    std::vector<int> tableIDs() const;

    BaseTableModel* table(int tableID) const;
    BaseTableModel* newTable();
    BaseTableModel* addTable(const std::string& tableName);
    BaseTableModel*
    convertColumnToTable(BaseTableModel* sourceTable,
                         int sourceColumnID,
                         const std::string& newTableName);
    BaseTableModel*
    duplicateTable(int tableID, const std::string& newName);
    bool removeTable(int tableID);

    BaseTableModel* columnReferenceTable(int sourceTableID,
                                                 int sourceColumnID);

    // Slots to be connected to BaseTableModels
    void onTableColumnAdded(int tableID, int columnID, int referenceID);
    void onTableColumnRemoved(int tableID, int columnID);

protected:
    BaseBookModelPrivate* d;
    BaseBookModel(BaseBookModelPrivate* data);
    virtual BaseTableModel* newBaseTable(const BaseTableModel* src);

    friend class BaseBookModelPrivate;
};

#endif // BASEBOOKMODEL_H
