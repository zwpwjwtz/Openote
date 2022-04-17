#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractItemModel>
#include "basetablemodel.h"


class TableModelPrivate;

class TableModel : public QAbstractItemModel, public BaseTableModel
{
    Q_OBJECT

public:
    explicit TableModel(QObject *parent = nullptr);
    TableModel(const TableModel &src);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant& value, int role = Qt::EditRole) override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index,
                  int role = Qt::DisplayRole) const override;
    QVariant nativeData(int rowIndex, int columnIndex) const;
    QString referenceData(int rowIndex, int columnIndex) const;

    // Editable:
    void clearColumn(int columnIndex);
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Add data:
    bool insertRow(int row);
    bool insertColumn(int column, const QString& name,
                      ColumnType type, int referenceTableID = 0);
    int newRow();
    int newColumn(const std::string& name,
                  ColumnType columnType,
                  int referenceID = 0);

    // Duplicate data:
    bool duplicateRow(int row);
    bool duplicateColumn(int column, const std::string &newName);

    // Remove data:
    void clear();
    bool removeRows(int row, int count,
                    const QModelIndex& parent = QModelIndex()) override;
    bool removeColumns(int column, int count,
                       const QModelIndex& parent = QModelIndex()) override;

protected:
    TableModelPrivate* d;
};

#endif // TABLEMODEL_H
