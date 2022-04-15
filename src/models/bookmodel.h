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
    QList<int> tableIDList() const;
    TableModel* table(int tableID) const;
    TableModel* addTable(const QString& tableName);
    TableModel* convertColumnToTable(BaseTableModel* sourceTable,
                                     int sourceColumnID,
                                     const QString& newTableName);
    TableModel* duplicateTable(int tableID, const QString& newName);
    TableModel* columnReferenceTable(int sourceTableID, int sourceColumnID);
    bool removeTable(int tableID);

    QString getTableName(int tableID) const;
    bool setTableName(int tableID, const QString& newName);

    BookIndex find(QString text,
                   BookIndex startIndex,
                   bool forward = true,
                   bool inAllTables = true,
                   bool caseSensitive = false);

protected:
    BookModelPrivate* d_ptr;
};

#endif // BOOKMODEL_H
