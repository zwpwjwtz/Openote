#ifndef BOOKVIEW_H
#define BOOKVIEW_H

#include <QTabWidget>
#include <QMap>
#include "models/bookmodel.h"


class QTableView;
class QHeaderView;
class DialogColumnAdd;
class BookModel;

class BookView : public QTabWidget
{
    Q_OBJECT
public:
    explicit BookView(QWidget *parent = nullptr);

    void clear();
    bool loadBook(const QString& path);
    bool loadDefaultBook();
    bool saveBook(const QString& path = "");

    QString currentPath() const;
    void setPath(const QString& newPath);

    bool modified() const;

public slots:
    bool addColumn();
    bool deleteColumn();
    bool duplicateColumn();
    bool renameColumn();

    bool addRow();
    bool deleteRow();
    bool duplicateRow();

    bool addTable();
    bool deleteTable();
    bool duplicateTable();
    bool renameTable();

private:
    struct BookIndex
    {
        int table;
        int column;
        int row;
        bool isValid()
        { return table >= 0 && column >= 0 && row >= 0; }
    };

    BookModel book;
    DialogColumnAdd* dialogColumnAdd;
    bool isModified;
    BookIndex currentBookIndex;

    int getTableID(int tableIndex) const;
    int getTableIndex(int tableID) const;
    BookIndex getCurrentIndex() const;

    QString columnHeader(int tableIndex, int columnIndex) const;
    bool setColumnHeader(const QString& text, int tableIndex, int columnIndex);

private slots:
    void onDialogColumnAddFinished(int result);
    void onTableDataChanged();
};

#endif // BOOKVIEW_H
