#ifndef BOOKVIEW_P_H
#define BOOKVIEW_P_H

#include <QObject>
#include "models/bookmodel.h"


class BookModel;
class ClipboardModel;
class TableView;
class ColumnReferenceDelegate;
class DialogColumnAdd;
class DialogFind;
class BookContextMenu;
class BookActionDispatcher;
class BookViewTabbar;

class BookView;

class BookViewPrivate : public QObject
{
    Q_OBJECT
public:
    struct BookIndex
    {
        int table = -1;
        int column = -1;
        int row = -1;
        bool operator ==(const BookIndex& src)
        { return table == src.table && column == src.column && row == src.row; }
        bool isValid()
        { return table >= 0 && column >= 0 && row >= 0; }
    };

    bool isModified;
    BookModel book;
    BookViewTabbar* tabBar;

    BookViewPrivate(BookView* parent = nullptr);
    ~BookViewPrivate();
    
    int getTableID(int tableIndex) const;
    int getTableIndex(int tableID) const;

    BookIndex getCurrentIndex();

    void bindTableModel(const TableModel* model);
    void bindTableView(TableView *table, TableModel* model);

    QString columnHeader(int tableIndex, int columnIndex) const;
    bool setColumnHeader(const QString& text, int tableIndex, int columnIndex);
    
    void findText(QString text, bool forward, bool inAllTables);

    ClipboardModel* getClipboard();
    BookContextMenu* getContextMenu();
    DialogFind* getFindDialog();
    DialogColumnAdd* getColumnAddDialog();
    
protected:
    BookView* q_ptr;
    BookIndex lastIndex;
    ClipboardModel* clipboard;
    DialogFind* dialogFind;
    DialogColumnAdd* dialogColumnAdd;
    ColumnReferenceDelegate* referenceDelegate;
    BookContextMenu* contextMenu;
    BookActionDispatcher* actionDispatcher;
    
protected slots:
    void onDialogColumnAddFinished(int result);
    void onDialogFindStart(QString text);
    void onTableDataChanged();
    void onColumnHeaderRightClicked(int index);
    void onColumnHeaderDoubleClicked(int index);
    void onGridRightClicked(int rowIndex, int columnIndex);
    void onTabCurrentIndexChanged(int tableIndex);
    void onTabBarDoubleClicked(int index);
};

#endif // BOOKVIEW_P_H
