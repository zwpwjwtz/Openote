#include "tablemodel.h"
#include "tablemodel_p.h"
#include "OpenTable/ontable_p.h"
#include "OpenTable/ontableintcolumn.h"
#include "OpenTable/ontabledoublecolumn.h"
#include "OpenTable/ontablestringcolumn.h"
#include "OpenTable/ontableintlistcolumn.h"


TableModel::TableModel(QObject *parent)
    : QAbstractItemModel(parent),
      ONTable (new TableModelPrivate())
{
    d = dynamic_cast<TableModelPrivate*>(ONTable::d_ptr);
}

TableModel::TableModel(const TableModel &src) :
    ONTable (new TableModelPrivate(*src.d))
{
    d = dynamic_cast<TableModelPrivate*>(ONTable::d_ptr);
}

TableModel* TableModel::clone(const TableModel& src)
{
    return new TableModel(src);
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();
    int columnID = ONTable::d_ptr->columnIDList[section];
    return QString::fromStdString(columnName(columnID));
}

bool TableModel::setHeaderData(int section, Qt::Orientation orientation,
                               const QVariant& value, int role)
{
    if (value != headerData(section, orientation, role))
    {
        int columnID = ONTable::d_ptr->columnIDList[section];
        setColumnName(columnID, value.toString().toStdString());
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}

QModelIndex TableModel::index(int row, int column,
                              const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return createIndex(row, column);
}

QModelIndex TableModel::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

int TableModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return countRow();
}

int TableModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return countColumn();
}

QVariant TableModel::data(const QModelIndex& index, int role) const
{
    if (!(index.isValid() &&
         (role == Qt::DisplayRole || role == Qt::EditRole)))
        return QVariant();

    int rowID = d->getRowID(index.row());
    int columnID = ONTable::d_ptr->columnIDList[index.column()];
    switch (ONTable::d_ptr->columnTypeIDList[index.column()])
    {
        case ColumnType::Integer:
            return readInt(rowID, columnID);
        case ColumnType::Double:
            return readDouble(rowID, columnID);
        case ColumnType::String:
            return QString::fromStdString(readString(rowID, columnID));
        case ColumnType::IntegerList:
        {
            auto rawList = readIntList(rowID, columnID);
            return QVariant::fromValue<QVector<int>>(
                        QVector<int>(rawList.cbegin(), rawList.cend()));
        }
        default:
            return QVariant();
    }
}

bool TableModel::setData(const QModelIndex& index,
                         const QVariant& value, int role)
{
    if (!(index.isValid() && role == Qt::EditRole))
        return false;

    bool successful;
    int rowID = d->getRowID(index.row());
    int columnID = ONTable::d_ptr->columnIDList[index.column()];
    switch (ONTable::d_ptr->columnTypeIDList[index.column()])
    {
        case ColumnType::Integer:
            successful = modify(rowID, columnID, value.toInt());
            break;
        case ColumnType::Double:
            successful = modify(rowID, columnID, value.toDouble());
            break;
        case ColumnType::String:
            successful = modify(rowID, columnID,
                                value.toString().toStdString());
            break;
        case ColumnType::IntegerList:
        {
            QStringList strings = value.toStringList();
            std::vector<int> integers;
            for (int i=0; i<strings.count(); i++)
                integers.push_back(strings[i].toInt());
            successful = modify(rowID, columnID, integers);
            break;
        }
        default:
            successful = false;
    }

    if (successful)
    {
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

Qt::ItemFlags TableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool TableModel::insertRows(int row, int count, const QModelIndex& parent)
{
    beginInsertRows(parent, row, row + count - 1);

    // Ignore the row parameter, as only appending operation is supported
    for (int i=0; i<count; i++)
        newRow();

    endInsertRows();
    return true;
}

bool TableModel::insertColumns(int column, int count,
                               const QModelIndex& parent)
{
    beginInsertColumns(parent, column, column + count - 1);

    for (int i=0; i<count; i++)
    {
        // No column type specified; default to String type
        newColumn("New column", ColumnType::String);
    }

    endInsertColumns();
    return true;
}

int TableModel::newRow()
{
    int rowIndex = countRow();
    beginInsertRows(QModelIndex(), rowIndex, rowIndex);
    int rowID = ONTable::newRow();
    endInsertRows();
    return rowID;
}

int TableModel::newColumn(const std::string &name, ColumnType columnType)
{
    int columnIndex = countColumn();
    beginInsertColumns(QModelIndex(), columnIndex, columnIndex);
    int columnID = ONTable::newColumn(name, columnType);
    endInsertColumns();
    return columnID;
}

bool TableModel::duplicateRow(int row)
{
    // TODO: duplicate the content of the raw
    return true;
}

bool TableModel::duplicateColumn(int column, const QString& newName)
{
    int columnIndex = countColumn();
    beginInsertColumns(QModelIndex(), columnIndex, columnIndex);

    ONTableColumn* oldColumn = ONTable::d_ptr->columnList[column];
    ONTableColumn* newColumn;
    switch (oldColumn->typeID)
    {
        case ColumnType::Integer:
            newColumn = new ONTableIntColumn(
                            *(ONTableIntColumn*)oldColumn);
            break;
        case ColumnType::Double:
            newColumn = new ONTableDoubleColumn(
                            *(ONTableDoubleColumn*)oldColumn);
            break;
        case ColumnType::String:
            newColumn = new ONTableStringColumn(
                            *(ONTableStringColumn*)oldColumn);
            break;
        case ColumnType::IntegerList:
            newColumn = new ONTableIntListColumn(
                            *(ONTableIntListColumn*)oldColumn);
            break;
        default:
            newColumn = nullptr;
            endInsertColumns();
            return false;
    }

    // Assuming increasing ID of column in the list
    int availableID = d->columnList.size() > 0 ?
                      d->columnList.back()->ID + 1 :
                      1;
    newColumn->ID = availableID;
    d->columnList.push_back(newColumn);
    d->columnIDList.push_back(availableID);
    d->columnTypeIDList.push_back(newColumn->typeID);
    d->columnNameList.push_back(newName.toStdString());

    endInsertColumns();
    return true;
}

bool TableModel::removeRows(int row, int count,
                            const QModelIndex& parent)
{
    beginRemoveRows(parent, row, row + count - 1);

    int rowID;
    for (int i=0; i<count; i++)
    {
        rowID = d->getRowID(row);
        ONTable::removeRow(rowID);
    }

    endRemoveRows();
    return true;
}

bool TableModel::removeColumns(int column, int count,
                               const QModelIndex& parent)
{
    beginRemoveColumns(parent, column, column + count - 1);

    for (int i=0; i<count; i++)
        ONTable::removeColumn(ONTable::d_ptr->columnIDList[column]);

    endRemoveColumns();
    return true;
}

TableModelPrivate::TableModelPrivate(const TableModelPrivate& src) :
    ONTablePrivate (src)
{}

int TableModelPrivate::getRowID(int rowIndex) const
{
    // The order of rows are fixed and does not allow changes
    // So just iterate over the IDList and take the ID located
    // at index rowIndex would be fine
    int count = 0;
    auto i = IDList.cbegin();
    while (i != IDList.cend())
    {
        if (count == rowIndex)
            break;
        count++;
        i++;
    }
    if (i == IDList.cend())
        return 0;
    else
        return *i;
}
