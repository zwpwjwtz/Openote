#ifndef BOOKCONTEXTMENU_H
#define BOOKCONTEXTMENU_H

#include <QObject>
#include <QPoint>


class QMenu;
class QAction;

class BookContextMenu : public QObject
{
    Q_OBJECT

public:
    enum MenuType
    {
        NullMenu = 0,
        TableMenu = 1,
        ColumnMenu = 2,
        RowMenu = 3,
        GridMenu = 4
    };

    enum ActionType
    {
        NullAction = 0,
        Adding = 1,
        Deleting = 2,
        Duplicating = 3,
        Renaming = 4,
        InsertingAhead = 5,
        InsertingAfter = 6,
        AddingColumn = 65,
        AddingRow = 66,
        DeletingColumn = 67,
        DeletingRow = 68,
        DuplicatingColumn = 69,
        DuplicatingRow = 70
    };

    explicit BookContextMenu(QObject* parent = nullptr);
    ~BookContextMenu();

public slots:
    void setMenuParent(QWidget* parent);

    void showTableMenu(QPoint pos = QPoint());
    void showColumnMenu(QPoint pos = QPoint());
    void showRowMenu(QPoint pos = QPoint());
    void showGridMenu(QPoint pos = QPoint());

    void setTableIsActive(bool active = true);
    void setRowIsActive(bool active = true);
    void setColumnIsActive(bool active = true);
    void setGridIsActive(bool active = true);

signals:
    void itemClicked(BookContextMenu::MenuType menu,
                     BookContextMenu::ActionType action);

private:
    QWidget* menuParent;
    QList<QMenu*> menuList;
    bool hasActiveTable;
    bool hasActiveColumn;
    bool hasActiveRow;

    QMenu* getMenu(MenuType type);
    void addAction(QMenu* menu, QString text, int actionID);
    void addSeparator(QMenu* menu);

private slots:
    void onItemClicked(QAction* action);
};

#endif // BOOKCONTEXTMENU_H
