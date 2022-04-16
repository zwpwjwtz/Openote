#ifndef BASEBOOKMODEL_H
#define BASEBOOKMODEL_H

#include "OpenTable/onbook.h"
#include "basetablemodel.h"


class BaseBookModelPrivate;

class BaseBookModel : public ONBook
{
public:
    BaseBookModel();

    bool load();

    BaseTableModel* table(int tableIndex) const;
    BaseTableModel* newTable();
    BaseTableModel* addTable(const std::string& tableName);
    BaseTableModel*
    convertColumnToTable(BaseTableModel* sourceTable,
                         int sourceColumnIndex,
                         const std::string& newTableName);
    BaseTableModel* duplicateTable(int tableIndex, const std::string& newName);
    bool removeTable(int tableIndex);

    std::string tableName(int tableIndex) const;
    bool setTableName(int tableIndex, const std::string& newName);

    bool setColumnReference(int sourceTableIndex,
                            int sourceColumnIndex,
                            int targetTableIndex);
    void removeColumnReference(int tableIndex, int columnIndex);
    BaseTableModel* columnReferenceTable(int tableID, int columnIndex);

    // Slots to be connected to BaseTableModels
    void onTableColumnAdded(int tableID, int columnID, int referenceID);
    void onTableColumnRemoved(int tableID, int columnID);

protected:
    BaseBookModelPrivate* d;
    BaseBookModel(BaseBookModelPrivate* data);
};

#endif // BASEBOOKMODEL_H
