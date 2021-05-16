#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractItemModel>
#include "OpenTable/ontable.h"


class TableModelPrivate;

class TableModel : public QAbstractItemModel, public ONTable
{
    Q_OBJECT

public:
    explicit TableModel(QObject *parent = nullptr);
    TableModel(const TableModel &src);

    static TableModel* clone(const TableModel &src);

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

    int rowID(int row) const;
    int columnID(int column) const;

    QVariant data(const QModelIndex& index,
                  int role = Qt::DisplayRole) const override;
    QVariant nativeData(int rowIndex, int columnIndex) const;
    QString referenceData(int rowIndex, int columnIndex) const;

    // Editable:
    void clearColumn(int columnID) override;
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole) override;
    bool setColumnReference(int columnIndex, int referenceID);

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Add data:
    bool insertRows(int row, int count,
                    const QModelIndex& parent = QModelIndex()) override;
    bool insertColumns(int column, int count,
                       const QModelIndex& parent = QModelIndex()) override;
    int newRow() override;
    int newColumn(const std::string& name, ColumnType columnType) override;
    int newColumn(const std::string &name, ColumnType columnType,
                  int referenceID);

    // Dupliate data:
    bool duplicateRow(int row);
    bool duplicateColumn(int column, const QString &newName);

    // Remove data:
    void clear() override;
    bool removeRows(int row, int count,
                    const QModelIndex& parent = QModelIndex()) override;
    bool removeColumns(int column, int count,
                       const QModelIndex& parent = QModelIndex()) override;

    bool load() override;

protected:
    TableModelPrivate* d;

signals:
    void columnAdded(int tableID, int columnID, int referenceID);
    void columnRemoved(int tableID, int columnID);
};

#endif // TABLEMODEL_H
