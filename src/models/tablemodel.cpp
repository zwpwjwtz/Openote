#include "tablemodel.h"
#include "tablemodel_p.h"
#include "bookmodel.h"
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
    QAbstractItemModel (src.QObject::parent()),
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

int TableModel::rowID(int row) const
{
    return d->getRowID(row);
}

int TableModel::columnID(int column) const
{
    return d->columnIDList[column];
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
    BookModel* parentBook = dynamic_cast<BookModel*>(QAbstractItemModel::parent());
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
    const TableModel* targetTable =
                        parentBook->columnReferenceTable(ID, columnID);

    // Use the first column in the referred table as data to display
    if (targetTable->d->columnIDList.size() < 1)
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

bool TableModel::setData(const QModelIndex& index,
                         const QVariant& value, int role)
{
    if (!(index.isValid() && role == Qt::EditRole))
        return false;

    bool successful = true;
    int rowID = d->getRowID(index.row());
    int columnID = ONTable::d_ptr->columnIDList[index.column()];
    switch (ONTable::d_ptr->columnTypeIDList[index.column()])
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

bool TableModel::setColumnReference(int columnID, int referenceID)
{
    int columnIndex = d->getColumnIndexByID(columnID);
    if (columnIndex < 0)
        return false;

    d->columnReferenceIDList[columnIndex] = referenceID;
    return true;
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
    return newColumn(name, columnType, 0);
}

int TableModel::newColumn(const std::string &name, ColumnType columnType,
                          int referenceID)
{
    int columnIndex = countColumn();
    beginInsertColumns(QModelIndex(), columnIndex, columnIndex);

    int columnID = ONTable::newColumn(name, columnType);
    d->columnReferenceIDList.push_back(referenceID);
    emit columnAdded(ID, columnID, referenceID);

    endInsertColumns();
    return columnID;
}

bool TableModel::duplicateRow(int row)
{
    int rowIndex = countRow();
    beginInsertRows(QModelIndex(), rowIndex, rowIndex);

    int oldRowID = d->getRowID(row);
    int newRowID = ONTable::newRow();
    int columnCount = d->columnList.size();
    for (int i=0; i<columnCount; i++)
    {
        ONTableColumn* column = d->columnList[i];
        switch (d->columnTypeIDList[i])
        {
            case ColumnType::Integer:
                ((ONTableIntColumn*)column)->duplicate(oldRowID, newRowID);
                break;
            case ColumnType::Double:
                ((ONTableDoubleColumn*)column)->duplicate(oldRowID, newRowID);
                break;
            case ColumnType::String:
                ((ONTableStringColumn*)column)->duplicate(oldRowID, newRowID);
                break;
            case ColumnType::IntegerList:
                ((ONTableIntListColumn*)column)->duplicate(oldRowID, newRowID);
                break;
            default:;
        }
    }
    endInsertRows();
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
    d->columnReferenceIDList.push_back(d->columnReferenceIDList[column]);
    emit columnAdded(ID, newColumn->ID, d->columnReferenceIDList[column]);

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

    int columnID;
    for (int i=0; i<count; i++)
    {
        columnID = ONTable::d_ptr->columnIDList[column];
        ONTable::removeColumn(columnID);
        d->columnReferenceIDList.removeAt(column);
        emit columnRemoved(ID, columnID);
    }

    endRemoveColumns();
    return true;
}

bool TableModel::load()
{
    if (!ONTable::load())
        return false;

    // Initialize columnReferenceIDList with zeros
    // These placeholders are necessary for the following
    // loading process of BookModel, because only columns
    // with reference can update their reference IDs in this list
    int count = d->columnList.size();
    d->columnReferenceIDList.reserve(count);
    while (count-- > 0)
        d->columnReferenceIDList.push_back(0);
    return true;
}

TableModelPrivate::TableModelPrivate(const TableModelPrivate& src) :
    ONTablePrivate (src)
{
    columnReferenceIDList = src.columnReferenceIDList;
}

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
