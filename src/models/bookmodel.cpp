#include <sstream>
#include "bookmodel.h"
#include "bookmodel_p.h"
#include "tablemodel.h"
#include "bookmodel_p.h"


BookModel::BookModel(QObject *parent) :
    QObject(parent)
{
    d_ptr = new BookModelPrivate(this);
}

BookModel::~BookModel()
{
    delete d_ptr;
}

void BookModel::clear()
{
    d_ptr->clear();
}

bool BookModel::load()
{
    return d_ptr->load();
}

bool BookModel::save()
{
    return d_ptr->save();
}

QString BookModel::path() const
{
    return QString::fromStdString(d_ptr->bindingDirectory());
}

bool BookModel::setPath(const QString& path)
{
    if (!path.isEmpty())
        return d_ptr->setBindingDirectory(path.toStdString());
    d_ptr->clearBindingDirectory();
    return true;
}

int BookModel::tableCount() const
{
    return d_ptr->count();
}

TableModel* BookModel::table(int tableIndex) const
{
    return static_cast<TableModel*>(d_ptr->table(tableIndex));
}

TableModel* BookModel::addTable(const QString& tableName)
{
    return static_cast<TableModel*>(d_ptr->addTable(tableName.toStdString()));
}

TableModel* BookModel::convertColumnToTable(int sourceTableIndex,
                                            int sourceColumnIndex,
                                            const QString& newTableName)
{
    return static_cast<TableModel*>
                    (d_ptr->convertColumnToTable(sourceTableIndex,
                                                 sourceColumnIndex,
                                                 newTableName.toStdString()));
}

TableModel* BookModel::duplicateTable(int tableIndex, const QString& newName)
{
    return static_cast<TableModel*>
                (d_ptr->duplicateTable(tableIndex, newName.toStdString()));
}

TableModel* BookModel::columnReferenceTable(int sourceTableID,
                                            int sourceColumnIndex)
{
    return static_cast<TableModel*>
            (d_ptr->columnReferenceTable(sourceTableID, sourceColumnIndex));
}

bool BookModel::removeTable(int tableIndex)
{
    return d_ptr->removeTable(tableIndex);
}

QString BookModel::getTableName(int tableIndex) const
{
    return QString::fromStdString(d_ptr->tableName(tableIndex));
}

bool BookModel::setTableName(int tableIndex, const QString& newName)
{
    return d_ptr->setTableName(tableIndex, newName.toStdString());
}

BookIndex BookModel::find(QString text, BookIndex startIndex,
                          bool forward, bool inAllTables, bool caseSensitive)
{
    BookIndex searchIndex = startIndex;

    // Find text in the following order: row->column->table
    bool found = false;
    TableModel* currentTable = static_cast<TableModel*>(table(searchIndex.table));
    QModelIndex tableIndex;
    int rowCount = currentTable->countRow();
    int columnCount = currentTable->countColumn();
    Qt::CaseSensitivity caseSensitivity = caseSensitive ?
                                          Qt::CaseSensitivity::CaseSensitive :
                                          Qt::CaseSensitivity::CaseInsensitive;
    while (true)
    {
        if (forward)
        {
            // Advance to the next row
            searchIndex.row++;
            if (searchIndex.row >= rowCount)
            {
                // Step into the next column
                searchIndex.row = 0;
                searchIndex.column++;
            }
            if (searchIndex.column >= columnCount)
            {
                searchIndex.column = 0;
                if (inAllTables)
                {
                    // Step into the next table
                    searchIndex.table++;
                    if (searchIndex.table >= tableCount())
                        searchIndex.table = 0;

                    currentTable =
                            static_cast<TableModel*>(table(searchIndex.table));
                    rowCount = currentTable->countRow();
                    columnCount = currentTable->countColumn();
                }
            }
        }
        else
        {
            // Regress to the previous row
            searchIndex.row--;
            if (searchIndex.row < 0)
            {
                // Step back to the previous column
                searchIndex.row = rowCount - 1;
                searchIndex.column--;
            }
            if (searchIndex.column < 0)
            {
                searchIndex.column = columnCount - 1;
                if (inAllTables)
                {
                    // Step back to the previous table
                    searchIndex.table++;
                    if (searchIndex.table >= tableCount())
                        searchIndex.table = 0;

                    currentTable =
                            static_cast<TableModel*>(table(searchIndex.table));
                    rowCount = currentTable->countRow();
                    columnCount = currentTable->countColumn();

                    searchIndex.column = columnCount - 1;
                    searchIndex.row = rowCount - 1;
                }
            }
        }

        // Compare the displayed content of the grid with searched text
        tableIndex = currentTable->index(searchIndex.row, searchIndex.column);
        if (currentTable->data(tableIndex, Qt::DisplayRole)
                        .toString().contains(text, caseSensitivity))
        {
            found = true;
            break;
        }

        if (searchIndex == startIndex)
        {
            // Reach the starting position: no match found
            searchIndex.reset();
            break;
        }
    }

    return searchIndex;
}

BookModelPrivate::BookModelPrivate(BookModel* parent)
{
    q_ptr = parent;
}

TableModel* BookModelPrivate::newTable()
{
    return new TableModel(q_ptr);
}
