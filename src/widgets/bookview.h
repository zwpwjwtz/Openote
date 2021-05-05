#ifndef BOOKVIEW_H
#define BOOKVIEW_H

#include <QTabWidget>
#include <QMap>
#include "OpenTable/onbook.h"


class QStandardItemModel;
class QTableView;
class QHeaderView;

class BookView : public QTabWidget
{
    Q_OBJECT
public:
    explicit BookView(QWidget *parent = nullptr);

    void clear();
    bool loadBook(const QString& path);
    bool loadDefaultBook();
    bool saveBook(const QString& path = "");

    bool modified() const;

private:
    QList<QStandardItemModel*> modelTables;
    ONBook book;
    bool isModified;

    int getTableIndex(int tableID) const;
    int getTableViewIndex(int tableID) const;
    int getColumnViewIndex(int tableID, int columnID) const;
    bool setColumnHeader(const QString& text, int columnID);
};

#endif // BOOKVIEW_H
