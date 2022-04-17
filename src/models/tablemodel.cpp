#include "tablemodel.h"
#include "tablemodel_p.h"
#include "bookmodel.h"


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
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Vertical)
        return d->rowHeaderPlaceholder;
    else
        return QString::fromStdString(columnName(section));
}

bool TableModel::setHeaderData(int section, Qt::Orientation orientation,
                               const QVariant& value, int role)
{
    if (value != headerData(section, orientation, role))
    {
        setColumnName(section, value.toString().toStdString());
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
    switch (d->columnTypeIDList[columnIndex])
    {
        case ColumnType::Integer:
            return readInt(rowIndex, columnIndex);
        case ColumnType::Double:
            return readDouble(rowIndex, columnIndex);
        case ColumnType::String:
            return QString::fromStdString(readString(rowIndex, columnIndex));
        case ColumnType::IntegerList:
        {
            auto rawList = readIntList(rowIndex, columnIndex);
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

    // Collect the IDs of referred rows in the specified position
    std::vector<int> referredRowIDs;
    switch (d->columnTypeIDList[columnIndex])
    {
        case ColumnType::Integer:
            referredRowIDs.push_back(readInt(rowIndex, columnIndex));
            break;
        case ColumnType::IntegerList:
            referredRowIDs = readIntList(rowIndex, columnIndex);
            break;
        default:
            return targetValue;
    }

    // Get the referred table
    const TableModel* targetTable = static_cast<TableModel*>
                            (parentBook->columnReferenceTable(ID, columnIndex));

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
                    targetTable->ONTable::readInt(*i, targetColumnID)));
            break;
        case ColumnType::Double:
            targetValue.append(QString::number(
                    targetTable->ONTable::readDouble(*i, targetColumnID)));
            break;
        case ColumnType::String:
            targetValue.append(QString::fromStdString(
                    targetTable->ONTable::readString(*i, targetColumnID)));
            break;
        case ColumnType::IntegerList:
        {
            auto rawList =
                    targetTable->ONTable::readIntList(*i, targetColumnID);
            for (auto j=rawList.cbegin(); j!=rawList.cend(); j++)
                targetValue.append(',').append(QString::number(*j));
            break;
        }
        default:;
        }
    }
    return targetValue;
}

void TableModel::clearColumn(int columnIndex)
{
    BaseTableModel::clearColumn(columnIndex);
    if (rowCount() > 0)
        emit dataChanged(index(0, columnIndex),
                         index(rowCount() - 1, columnIndex),
                         QVector<int>(Qt::EditRole));
}

bool TableModel::setData(const QModelIndex& index,
                         const QVariant& value, int role)
{
    if (!(index.isValid() && role == Qt::EditRole))
        return false;

    int row = index.row(), column = index.column();
    bool successful = true;
    switch (BaseTableModel::d_ptr->columnTypeIDList[column])
    {
        case ColumnType::Integer:
            if (readInt(row, column) != value.toInt())
                successful = modify(row, column, value.toInt());
            break;
        case ColumnType::Double:
            if (readDouble(row, column) != value.toDouble())
                successful = modify(row, column, value.toDouble());
            break;
        case ColumnType::String:
        {
            std::string rawString = value.toString().toStdString();
            if (readString(row, column) != rawString)
                successful = modify(row, column, rawString);
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
            if (readIntList(row, column) != integers)
                successful = modify(row, column, integers);
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

bool TableModel::insertRow(int row)
{
    beginInsertRows(QModelIndex(), row, row);
    BaseTableModel::insertRow(row);
    endInsertRows();
    return true;
}

bool TableModel::insertColumn(int column, const QString& name,
                              ColumnType type, int referenceTableID)
{
    beginInsertColumns(QModelIndex(), column, column);
    BaseTableModel::insertColumn(column, name.toStdString(),
                                 type, referenceTableID);
    endInsertColumns();
    return true;
}

int TableModel::newRow()
{
    int rowIndex = countRow();
    beginInsertRows(QModelIndex(), rowIndex, rowIndex);
    rowIndex = BaseTableModel::newRow();
    endInsertRows();
    return rowIndex;
}

int TableModel::newColumn(const std::string &name, ColumnType columnType,
                          int referenceID)
{
    int columnIndex = countColumn();
    beginInsertColumns(QModelIndex(), columnIndex, columnIndex);
    int columnID = BaseTableModel::newColumn(name, columnType, referenceID);
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
    ONTable::clear();
    emit layoutChanged();
}

bool TableModel::removeRows(int row, int count,
                            const QModelIndex& parent)
{
    beginRemoveRows(parent, row, row + count - 1);

    for (int i=0; i<count; i++)
        BaseTableModel::removeRow(row + i);

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
