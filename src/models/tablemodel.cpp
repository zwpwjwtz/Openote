#include "tablemodel.h"
#include "tablemodel_p.h"
#include "bookmodel.h"
#include "basetablemodel_p.h"


TableModel::TableModel(QObject *parent)
    : QAbstractItemModel(parent),
      BaseTableModel (new TableModelPrivate())
{
    d = static_cast<TableModelPrivate*>(BaseTableModel::d);
}

TableModel::TableModel(const TableModel &src)
    : QAbstractItemModel (src.QObject::parent()),
      BaseTableModel (new TableModelPrivate(*src.d))
{
    d = static_cast<TableModelPrivate*>(BaseTableModel::d);
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();
    int columnID = BaseTableModel::d_ptr->columnIDList[section];
    return QString::fromStdString(columnName(columnID));
}

bool TableModel::setHeaderData(int section, Qt::Orientation orientation,
                               const QVariant& value, int role)
{
    if (value != headerData(section, orientation, role))
    {
        int columnID = BaseTableModel::d_ptr->columnIDList[section];
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
    Q_UNUSED(index)
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

    if (d->columnReferenceIDList[index.column()] > 0)
    {
        if (role == Qt::DisplayRole)
            return referenceData(index.row(), index.column());
        else if (role == Qt::EditRole)
            return nativeData(index.row(), index.column());
        else
            return QVariant(); // TODO: use customized widgets to edit references
    }
    else
        return nativeData(index.row(), index.column());
}

QVariant TableModel::nativeData(int rowIndex, int columnIndex) const
{
    int rowID = d->getRowID(rowIndex);
    int columnID = d->columnIDList[columnIndex];
    switch (d->columnTypeIDList[columnIndex])
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
            QList<QVariant> valueList;
            valueList.reserve(rawList.size());
            for (auto i=rawList.cbegin(); i!=rawList.cend(); i++)
                valueList.push_back(*i);
            return valueList;
        }
        default:
            return QVariant();
    }
}

QString TableModel::referenceData(int rowIndex, int columnIndex) const
{
    QString targetValue;
    BookModel* parentBook =
                    static_cast<BookModel*>(QAbstractItemModel::parent());
    if (parentBook == nullptr)
        return targetValue;

    int rowID = d->getRowID(rowIndex);
    int columnID = d->columnIDList[columnIndex];

    // Collect the IDs of referred rows in the specified position
    std::vector<int> referredRowIDs;
    switch (d->columnTypeIDList[columnIndex])
    {
        case ColumnType::Integer:
            referredRowIDs.push_back(readInt(rowID, columnID));
            break;
        case ColumnType::IntegerList:
            referredRowIDs = readIntList(rowID, columnID);
            break;
        default:
            return targetValue;
    }

    // Get the referred table
    const TableModel* targetTable = static_cast<TableModel*>
                            (parentBook->columnReferenceTable(ID, columnID));

    // Use the first column in the referred table as data to display
    if (targetTable == nullptr || targetTable->d->columnIDList.size() < 1)
        return targetValue;
    int targetColumnID = targetTable->d->columnIDList[0];
    for (auto i=referredRowIDs.cbegin(); i!=referredRowIDs.cend(); i++)
    {
         if (i != referredRowIDs.cbegin())
             targetValue.append('|');
        switch (targetTable->d->columnTypeIDList[0])
        {
        case ColumnType::Integer:
            targetValue.append(QString::number(
                               targetTable->readInt(*i, targetColumnID)));
            break;
        case ColumnType::Double:
            targetValue.append(QString::number(
                               targetTable->readDouble(*i, targetColumnID)));
            break;
        case ColumnType::String:
            targetValue.append(QString::fromStdString(
                               targetTable->readString(*i, targetColumnID)));
            break;
        case ColumnType::IntegerList:
        {
            auto rawList = targetTable->readIntList(*i, targetColumnID);
            for (auto j=rawList.cbegin(); j!=rawList.cend(); j++)
                targetValue.append(',').append(QString::number(*j));
            break;
        }
        default:;
        }
    }
    return targetValue;
}

void TableModel::clearColumn(int columnID)
{
     BaseTableModel::clearColumn(columnID);
    if (rowCount() > 0)
    {
        int columnIndex = d->getColumnIndexByID(columnID);
        emit dataChanged(index(0, columnIndex),
                         index(rowCount() - 1, columnIndex),
                         QVector<int>(Qt::EditRole));
    }
}

bool TableModel::setData(const QModelIndex& index,
                         const QVariant& value, int role)
{
    if (!(index.isValid() && role == Qt::EditRole))
        return false;

    bool successful = true;
    int rowID = d->getRowID(index.row());
    int columnID = BaseTableModel::d_ptr->columnIDList[index.column()];
    switch (BaseTableModel::d_ptr->columnTypeIDList[index.column()])
    {
        case ColumnType::Integer:
            if (readInt(rowID, columnID) != value.toInt())
                successful = modify(rowID, columnID, value.toInt());
            break;
        case ColumnType::Double:
            if (readDouble(rowID, columnID) != value.toDouble())
                successful = modify(rowID, columnID, value.toDouble());
            break;
        case ColumnType::String:
        {
            std::string rawString = value.toString().toStdString();
            if (readString(rowID, columnID) != rawString)
                successful = modify(rowID, columnID, rawString);
            break;
        }
        case ColumnType::IntegerList:
        {
            int i;
            QList<QVariant> valueList = value.toList();
            if (valueList.isEmpty())
            {
                QStringList strings = value.toString().split(',');
                for (i=0; i<strings.count(); i++)
                    valueList.push_back(strings[i].toInt());
            }

            int length = valueList.length();
            std::vector<int> integers;
            integers.reserve(length);
            for (i=0; i<length; i++)
                integers.push_back(valueList[i].toInt());
            if (readIntList(rowID, columnID) != integers)
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
    int rowID = BaseTableModel::newRow();
    endInsertRows();
    return rowID;
}

int TableModel::newColumn(const std::string &name, ColumnType columnType)
{
    return newColumn(name, columnType, 0);
}

int TableModel::newColumn(const std::string &name, ColumnType columnType,
                          int referenceID)
{
    int columnIndex = countColumn();
    beginInsertColumns(QModelIndex(), columnIndex, columnIndex);

    int columnID = BaseTableModel::newColumn(name, columnType);
    d->columnReferenceIDList.push_back(referenceID);
    emit columnAdded(ID, columnID, referenceID);

    endInsertColumns();
    return columnID;
}

bool TableModel::duplicateRow(int row)
{
    int rowIndex = countRow();
    beginInsertRows(QModelIndex(), rowIndex, rowIndex);
    bool successful = BaseTableModel::duplicateRow(row);
    endInsertRows();
    return successful;
}

bool TableModel::duplicateColumn(int column, const std::string& newName)
{
    int columnIndex = countColumn();
    beginInsertColumns(QModelIndex(), columnIndex, columnIndex);
    bool successful = BaseTableModel::duplicateColumn(column, newName);
    endInsertColumns();
    return successful;
}

void TableModel::clear()
{
    BaseTableModel::clear();
    emit layoutChanged();
}

bool TableModel::removeRows(int row, int count,
                            const QModelIndex& parent)
{
    beginRemoveRows(parent, row, row + count - 1);

    int rowID;
    for (int i=0; i<count; i++)
    {
        rowID = d->getRowID(row);
        BaseTableModel::removeRow(rowID);
    }

    endRemoveRows();
    return true;
}

bool TableModel::removeColumns(int column, int count,
                               const QModelIndex& parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    for (int i=0; i<count; i++)
        BaseTableModel::removeColumn(column + i);
    endRemoveColumns();
    return true;
}

TableModelPrivate::TableModelPrivate(const TableModelPrivate& src) :
    BaseTableModelPrivate (src)
{}

TableModelPrivate::~TableModelPrivate()
{}
