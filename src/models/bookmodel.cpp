#include <QList>
#include <sstream>
#include "bookmodel.h"
#include "bookmodel_p.h"
#include "tablemodel.h"
#include "OpenTable/onbook_p.h"


BookModel::BookModel(QObject *parent) :
    QObject(parent),
    ONBook (new BookModelPrivate)
{
    d = dynamic_cast<BookModelPrivate*>(ONBook::d_ptr);
}

BookModel::~BookModel()
{
    clear();
}

void BookModel::clear()
{
    for (size_t i=0; i<d->tableList.size(); i++)
        delete d->tableList[i];
    d->tableList.clear();
    d->tableIDList.clear();
    d->tableNameList.clear();

    // Update the data of parent class
    ONBook::d_ptr->tableList.clear();
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
    // Assuming increasing ID of tables in the list
    int availableID = d->tableList.size() > 0 ?
                      d->tableList.back()->ID + 1 : 1;

    TableModel* newTable = new TableModel();
    newTable->ID = availableID;
    d->tableList.push_back(newTable);
    d->tableIDList.push_back(availableID);
    d->tableNameList.push_back(tableName.toStdString());

    // Update the data of parent class
    ONBook::d_ptr->tableList.push_back(newTable);

    return newTable;
}

TableModel* BookModel::duplicateTable(int tableID, const QString& newName)
{
    const TableModel* table = this->table(tableID);
    if (table == nullptr)
        return nullptr;

    // Assuming increasing ID of tables in the list
    int availableID = d->tableList.size() > 0 ?
                      d->tableList.back()->ID + 1 : 1;

    TableModel* newTable = new TableModel(*table);
    newTable->ID = availableID;
    d->tableList.push_back(newTable);
    d->tableIDList.push_back(availableID);
    d->tableNameList.push_back(newName.toStdString());

    // Update the data of parent class
    ONBook::d_ptr->tableList.push_back(newTable);

    return newTable;
}

bool BookModel::removeTable(int tableID)
{
    int index = d->getTableIndexByID(tableID);
    if (index < 0)
        return false;

    d->tableList.erase(d->tableList.begin() + index);
    d->tableIDList.erase(d->tableIDList.begin() + index);
    d->tableNameList.erase(d->tableNameList.begin() + index);

    // Update the data of parent class
    ONBook::d_ptr->tableList.erase(ONBook::d_ptr->tableList.begin() + index);

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

bool BookModelPrivate::loadTable(int tableID, const std::string& tableName)
{
    TableModel* table = new TableModel();
    tableList.push_back(table);
    if (!(table->setBindingDirectory(getTableDirectory(tableID)) &&
          table->load()))
        return false;

    table->ID = tableID;
    tableIDList.push_back(tableID);
    tableNameList.push_back(tableName);

    // Update the data of parent class
    ONBookPrivate::tableList.push_back(table);

    return true;
}
