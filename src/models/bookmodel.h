#ifndef BOOKMODEL_H
#define BOOKMODEL_H

#include <QObject>
#include "OpenTable/onbook.h"


class TableModel;
class BookModelPrivate;

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
    TableModel* duplicateTable(int tableID, const QString& newName);
    bool removeTable(int tableID);

    const TableModel* columnReferenceTable(int sourceTableID,
                                           int sourceColumnID);

    QString path() const;
    bool setPath(const QString& path);

    bool load();
    bool save();

protected:
    BookModelPrivate* d;
    BookModel(BookModelPrivate* data);

private slots:
    void onTableColumnAdded(int tableID, int columnID, int referenceID);
    void onTableColumnRemoved(int tableID, int columnID);
};

#endif // BOOKMODEL_H
