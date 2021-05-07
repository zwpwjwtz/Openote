#include <QList>
#include "bookmodel.h"
#include "bookmodel_p.h"
#include "tablemodel.h"
#include "OpenTable/onbook_p.h"


BookModel::BookModel(QObject *parent) : QObject(parent)
{
    d = new BookModelPrivate;
}

void BookModel::clear()
{
    ONBook::clear();
}

QList<int> BookModel::tableIDs() const
{
    QList<int> IDList;
    auto stdList = ONBook::tableIDs();
    return QList<int>(stdList.cbegin(), stdList.cend());
}

QString BookModel::tableName(int tableID) const
{
    return QString::fromStdString(ONBook::tableName(tableID));
}

bool BookModel::setTableName(int tableID, const QString& newName)
{
    return ONBook::setTableName(tableID, newName.toStdString());
}

TableModel* BookModel::table(int tableID) const
{
    return dynamic_cast<TableModel*>(ONBook::table(tableID));
}

TableModel* BookModel::addTable(const QString& tableName)
{
    TableModel* newTable = new TableModel();
    auto tableList = ONBook::d_ptr->tableList;
    tableList.push_back(*dynamic_cast<ONTable*>(newTable));

    // Assuming increasing ID of tables in the list
    int availableID = tableList.size() > 0 ? tableList.back().ID + 1 : 1;
    newTable->ID = availableID;
    ONBook::d_ptr->tableIDList.push_back(availableID);
    ONBook::d_ptr->tableNameList.push_back(tableName.toStdString());
    return newTable;
}

TableModel* BookModel::duplicateTable(int tableID, const QString& newName)
{
    ONTable* table = ONBook::table(tableID);
    if (table == nullptr)
        return nullptr;
    TableModel* newTable = addTable(newName);
    if (newName == nullptr)
        return nullptr;
    dynamic_cast<ONTable&>(*newTable) = *table;
    return newTable;
}

bool BookModel::removeTable(int tableID)
{
    ONBook::removeTable(tableID);
    return true;
}

QString BookModel::path() const
{
    return QString::fromStdString(ONBook::bindingDirectory());
}

bool BookModel::setPath(const QString& path)
{
    return ONBook::setBindingDirectory(path.toStdString());
}

bool BookModel::load()
{
    return ONBook::load();
}

bool BookModel::save()
{
    return ONBook::save();
}
