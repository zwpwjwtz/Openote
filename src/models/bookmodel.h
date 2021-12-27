#ifndef BOOKMODEL_H
#define BOOKMODEL_H

#include <QObject>
#include "OpenTable/onbook.h"


class TableModel;
class BookModelPrivate;

struct BookIndex
{
    int table = -1;
    int column = -1;
    int row = -1;
    bool operator ==(const BookIndex& src)
    { return table == src.table && column == src.column && row == src.row; }
    bool isValid()
    { return table >= 0 && column >= 0 && row >= 0; }
    void reset()
    { table = column = row = -1; }
};

class BookModel : public QObject, protected ONBook
{
    Q_OBJECT
public:
    explicit BookModel(QObject *parent = nullptr);
    virtual ~BookModel();

    void clear();

    int tableCount() const;
    QList<int> tableIDs() const;

    QString tableName(int tableID) const;
    bool setTableName(int tableID, const QString& newName);

    TableModel* table(int tableID) const;
    TableModel* addTable(const QString& tableName);
    TableModel* convertColumnToTable(TableModel *sourceTable,
                                     int sourceColumnID,
                                     const QString& newTableName);
    TableModel* duplicateTable(int tableID, const QString& newName);
    bool removeTable(int tableID);

    TableModel* columnReferenceTable(int sourceTableID, int sourceColumnID);

    QString path() const;
    bool setPath(const QString& path);

    bool load();
    bool save();

    BookIndex find(QString text,
                   BookIndex startIndex,
                   bool forward = true,
                   bool inAllTables = true,
                   bool caseSensitive = false);

protected:
    BookModelPrivate* d;
    BookModel(BookModelPrivate* data);

private slots:
    void onTableColumnAdded(int tableID, int columnID, int referenceID);
    void onTableColumnRemoved(int tableID, int columnID);
};

#endif // BOOKMODEL_H
