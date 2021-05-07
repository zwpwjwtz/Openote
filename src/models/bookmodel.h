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

    void clear();

    QList<int> tableIDs() const;

    QString tableName(int tableID) const;
    bool setTableName(int tableID, const QString& newName);

    TableModel* table(int tableID) const;
    TableModel* addTable(const QString& tableName);
    TableModel* duplicateTable(int tableID, const QString& newName);
    bool removeTable(int tableID);

    QString path() const;
    bool setPath(const QString& path);

    bool load();
    bool save();

signals:

protected:
    BookModelPrivate* d;
    BookModel(BookModelPrivate* data);
};

#endif // BOOKMODEL_H
