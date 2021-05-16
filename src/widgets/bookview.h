#ifndef BOOKVIEW_H
#define BOOKVIEW_H

#include <QTabWidget>
#include <QMap>
#include "models/bookmodel.h"


class BookModel;
class TableView;
class ColumnReferenceDelegate;
class DialogColumnAdd;
class BookContextMenu;
class BookActionDispatcher;

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
    bool columnToTable();

    bool addRow();
    bool deleteRow();
    bool duplicateRow();

    bool addTable();
    bool deleteTable();
    bool duplicateTable();
    bool renameTable();

protected:
    void mousePressEvent(QMouseEvent* event);

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
    ColumnReferenceDelegate* referenceDelegate;
    BookContextMenu* contextMenu;
    BookActionDispatcher* actionDispatcher;

    int getTableID(int tableIndex) const;
    int getTableIndex(int tableID) const;
    BookIndex getCurrentIndex() const;

    void bindTableModel(const TableModel* model);
    void bindTableView(TableView *table);

    QString columnHeader(int tableIndex, int columnIndex) const;
    bool setColumnHeader(const QString& text, int tableIndex, int columnIndex);

private slots:
    void onDialogColumnAddFinished(int result);
    void onTableDataChanged();
    void onColumnHeaderRightClicked(int index);
    void onColumnHeaderDoubleClicked(int index);
    void onGridRightClicked(int rowIndex, int columnIndex);
    void onTabBarDoubleClicked(int index);
};

#endif // BOOKVIEW_H
