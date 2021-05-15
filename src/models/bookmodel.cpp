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
    d->q_ptr = this;
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

int BookModel::tableCount() const
{
    return ONBook::count();
}

QList<int> BookModel::tableIDs() const
{
    auto stdList = ONBook::tableIDs();
    QList<int> IDList;
    IDList.reserve(stdList.size());
    for (auto i=stdList.cbegin(); i!=stdList.cend(); i++)
        IDList.push_back(*i);
    return IDList;
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

    TableModel* newTable = new TableModel(this);
    newTable->ID = availableID;
    d->tableList.push_back(newTable);
    d->tableIDList.push_back(availableID);
    d->tableNameList.push_back(tableName.toStdString());
    connect(newTable, SIGNAL(columnAdded(int, int, int)),
            this, SLOT(onTableColumnAdded(int, int, int)));
    connect(newTable, SIGNAL(columnRemoved(int, int)),
            this, SLOT(onTableColumnRemoved(int, int)));

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
    for (auto i=d->columnReference.begin(); i!=d->columnReference.cend(); i++)
    {
        if (i->first.first == tableID)
        {
            d->columnReference.insert(
                std::make_pair(std::make_pair(newTable->ID, i->first.second),
                               i->second));
        }
    }
    connect(newTable, SIGNAL(columnAdded(int, int, int)),
            this, SLOT(onTableColumnAdded(int, int, int)));
    connect(newTable, SIGNAL(columnRemoved(int, int)),
            this, SLOT(onTableColumnRemoved(int, int)));

    // Update the data of parent class
    ONBook::d_ptr->tableList.push_back(newTable);

    return newTable;
}

bool BookModel::removeTable(int tableID)
{
    int index = d->getTableIndexByID(tableID);
    if (index < 0)
        return false;

    delete d->tableList[index];
    d->tableList.erase(d->tableList.begin() + index);
    d->tableIDList.erase(d->tableIDList.begin() + index);
    d->tableNameList.erase(d->tableNameList.begin() + index);
    for (auto i=d->columnReference.begin(); i!=d->columnReference.cend(); i++)
    {
        if (i->first.first == tableID)
            d->columnReference.erase(i);
    }

    // Update the data of parent class
    ONBook::d_ptr->tableList.erase(ONBook::d_ptr->tableList.begin() + index);

    return true;
}

TableModel*
BookModel::columnReferenceTable(int sourceTableID, int sourceColumnID)
{
    int targetTableID = ONBook::columnReference(sourceTableID, sourceColumnID);
    if (targetTableID > 0)
        return d->tableList[d->getTableIndexByID(targetTableID)];
    else
        return nullptr;
}

QString BookModel::path() const
{
    return QString::fromStdString(ONBook::bindingDirectory());
}

bool BookModel::setPath(const QString& path)
{
    if (!path.isEmpty())
        return ONBook::setBindingDirectory(path.toStdString());
    ONBook::clearBindingDirectory();
    return true;
}

bool BookModel::load()
{
    if (!ONBook::load())
        return false;

    // Listen to tables' events
    for (auto j=d->tableList.cbegin(); j!=d->tableList.cend(); j++)
    {
        connect(*j, SIGNAL(columnAdded(int, int, int)),
                this, SLOT(onTableColumnAdded(int, int, int)));
        connect(*j, SIGNAL(columnRemoved(int, int)),
                this, SLOT(onTableColumnRemoved(int, int)));
    }

    // Notify all TableModel with column reference IDs
    TableModel* table = nullptr;
    for (auto i=d->columnReference.cbegin(); i!=d->columnReference.cend(); i++)
    {
        if (table == nullptr || table->ID != i->first.first)
        {
            table = d->tableList[d->getTableIndexByID(i->first.first)];
        }
        table->setColumnReference(i->first.second, i->second);
    }
    return true;
}

bool BookModel::save()
{
    return ONBook::save();
}

void BookModel::onTableColumnAdded(int tableID, int columnID, int referenceID)
{
    if (referenceID > 0)
        ONBook::setColumnReference(tableID, columnID, referenceID);
}

void BookModel::onTableColumnRemoved(int tableID, int columnID)
{
    ONBook::removeColumnReference(tableID, columnID);
}

bool BookModelPrivate::loadTable(int tableID, const std::string& tableName)
{
    TableModel* table = new TableModel(q_ptr);
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
