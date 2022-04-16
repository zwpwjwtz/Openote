#ifndef BOOKMODEL_H
#define BOOKMODEL_H

#include <QObject>
#include "tablemodel.h"
#include "bookindex.h"


class TableModel;
class BookModelPrivate;

class BookModel : public QObject
{
    Q_OBJECT
public:
    explicit BookModel(QObject *parent = nullptr);
    ~BookModel();

    void clear();

    bool load();
    bool save();

    QString path() const;
    bool setPath(const QString& path);

    int tableCount() const;
    TableModel* table(int tableIndex) const;
    TableModel* addTable(const QString& tableName);
    TableModel* convertColumnToTable(int sourceTableIndex,
                                     int sourceColumnIndex,
                                     const QString& newTableName);
    TableModel* duplicateTable(int tableIndex, const QString& newName);
    TableModel* columnReferenceTable(int sourceTableID, int sourceColumnIndex);
    bool removeTable(int tableIndex);

    QString getTableName(int tableIndex) const;
    bool setTableName(int tableIndex, const QString& newName);

    BookIndex find(QString text,
                   BookIndex startIndex,
                   bool forward = true,
                   bool inAllTables = true,
                   bool caseSensitive = false);

protected:
    BookModelPrivate* d_ptr;
};

#endif // BOOKMODEL_H
