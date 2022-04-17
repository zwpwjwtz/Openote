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

    bool bindBookModel(BaseBookModel *model);
    bool setColumnReference(int columnIndex, int referenceID);

    int rowID(int rowIndex) const;

    void clear(int rowIndex, int columnIndex);
    void clearRow(int rowIndex);
    void clearColumn(int columnIndex);

    int newRow();
    int newColumn(const std::string& name,
                  ColumnType columnType,
                  int referenceID = 0);

    int insertRow(int rowIndex);
    int insertColumn(int columnIndex,
                     const std::string& name,
                     ColumnType columnType,
                     int referenceID = 0);

    bool duplicateRow(int rowIndex);
    bool duplicateColumn(int columnIndex, const std::string& newName);

    void moveRow(int fromIndex, int toIndex);
    void moveColumn(int fromIndex, int toIndex);

    void removeRow(int rowIndex);
    void removeColumn(int columnIndex);

    std::string columnName(int columnIndex) const;
    void setColumnName(int columnIndex, const std::string& newName);

    ColumnType columnType(int columnIndex) const;
    bool setColumnType(int columnIndex, ColumnType newType);

    bool load();

    int readInt(int rowIndex, int columnIndex) const;
    double readDouble(int rowIndex, int columnIndex) const;
    std::string readString(int rowIndex, int columnIndex) const;
    std::vector<int> readIntList(int rowIndex, int columnIndex) const;

    template<typename T>
    bool modify(int rowIndex, int columnIndex, const T& value);

    template<typename T>
    std::list<int> insert(int columnIndex, const std::list<T>& valueList);

protected:
    BaseTableModelPrivate* d;

    BaseTableModel(BaseTableModelPrivate* data);
    int columnID(int columnIndex) const;
    int columnIndex(int columnID) const;

    friend class BaseBookModel;
};

#endif // BASETABLEMODEL_H
