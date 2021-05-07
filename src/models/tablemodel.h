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

    // Editable:
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Add data:
    bool insertRows(int row, int count,
                    const QModelIndex& parent = QModelIndex()) override;
    bool insertColumns(int column, int count,
                       const QModelIndex& parent = QModelIndex()) override;

    // Dupliate data:
    bool duplicateRow(int row);
    bool duplicateColumn(int column);

    // Remove data:
    bool removeRows(int row, int count,
                    const QModelIndex& parent = QModelIndex()) override;
    bool removeColumns(int column, int count,
                       const QModelIndex& parent = QModelIndex()) override;

private:
    TableModelPrivate* d;
};

#endif // TABLEMODEL_H
