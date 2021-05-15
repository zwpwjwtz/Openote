#include "bookcontextmenu.h"
#include <QMenu>

#define OPENOTE_BOOKMENU_MENU_COUNT    4


BookContextMenu::BookContextMenu(QObject* parent) : QObject(parent)
{
    menuParent = nullptr;
    for (int i=0; i<OPENOTE_BOOKMENU_MENU_COUNT; i++)
        menuList.push_back(nullptr);

    hasActiveTable = false;
    hasActiveColumn = false;
    hasActiveGrid = false;
}

BookContextMenu::~BookContextMenu()
{
    for (int i=0; i<menuList.count(); i++)
    {
        if (menuList[i] != nullptr)
            delete menuList[i];
    }
}

void BookContextMenu::setMenuParent(QWidget* parent)
{
    menuParent = parent;
}

void BookContextMenu::showTableMenu(QPoint pos)
{
    if (pos.isNull())
        pos = QCursor::pos();

    QMenu* menu = getMenu(MenuType::TableMenu);
    menu->clear();
    addAction(menu, tr("New table"), ActionType::Adding);
    if (hasActiveTable)
    {
        addAction(menu, tr("Delete table"), ActionType::Deleting);
        addAction(menu, tr("Duplicate table"), ActionType::Duplicating);
        addSeparator(menu);
        addAction(menu, tr("Rename table"), ActionType::Renaming);
    }
    menu->popup(pos);
}

void BookContextMenu::showColumnMenu(QPoint pos)
{
    if (pos.isNull())
        pos = QCursor::pos();

    QMenu* menu = getMenu(MenuType::ColumnMenu);
    menu->clear();
    addAction(menu, tr("New column"), ActionType::Adding);
    if (hasActiveColumn)
    {
        addAction(menu, tr("Delete column"), ActionType::Deleting);
        addAction(menu, tr("Duplicate column"), ActionType::Duplicating);
        addSeparator(menu);
        addAction(menu, tr("Rename column"), ActionType::Renaming);
    }
    menu->popup(pos);
}

void BookContextMenu::showRowMenu(QPoint pos)
{
    if (pos.isNull())
        pos = QCursor::pos();

    QMenu* menu = getMenu(MenuType::RowMenu);
    menu->clear();
    addAction(menu, tr("New row"), ActionType::Adding);
    if (hasActiveGrid)
    {
        addAction(menu, tr("Delete row"), ActionType::Deleting);
        addAction(menu, tr("Duplicate row"), ActionType::Duplicating);
    }
    menu->popup(pos);
}

void BookContextMenu::showGridMenu(QPoint pos)
{
    if (pos.isNull())
        pos = QCursor::pos();

    QMenu* menu = getMenu(MenuType::GridMenu);
    menu->clear();
    addAction(menu, tr("New row"), ActionType::AddingRow);
    if (hasActiveGrid)
    {
        addAction(menu, tr("Delete row"), ActionType::DeletingRow);
        addAction(menu, tr("Duplicate row"), ActionType::DuplicatingRow);
    }
    menu->popup(pos);
}

void BookContextMenu::setTableIsActive(bool hasFocus)
{
    hasActiveTable = hasFocus;
}

void BookContextMenu::setColumnIsActive(bool hasFocus)
{
    hasActiveColumn = hasFocus;
}

void BookContextMenu::setGridIsActive(bool hasFocus)
{
    hasActiveGrid = hasFocus;
    if (hasFocus)
        hasActiveTable = hasActiveColumn = true;
}

QMenu* BookContextMenu::getMenu(MenuType type)
{
    // The menus in the list have the same order as
    // their types in the enum MenuType
    int index = int(type) - 1;

    QMenu*& menu = menuList[index];
    if (menu == nullptr)
    {
        menu = new QMenu(menuParent);
        connect(menu, SIGNAL(triggered(QAction*)),
                this, SLOT(onItemClicked(QAction*)));
    }
    return menu;
}

void BookContextMenu::onItemClicked(QAction* action)
{
    // Find the parent menu that the action belongs to
    int i;
    for (i=0; i<menuList.count(); i++)
    {
        if (menuList[i] != nullptr &&
            menuList[i]->actions().contains(action))
            break;
    }
    if (i >= menuList.count())
        return;

    // Broadcast the event according to the menu type
    MenuType menuType = MenuType(i + 1);
    ActionType actionType = ActionType(action->data().toInt());
    emit itemClicked(menuType, actionType);
}

void BookContextMenu::addAction(QMenu* menu, QString text, int actionID)
{
    QAction* newAction = new QAction(text, this);
    newAction->setData(actionID);
    menu->addAction(newAction);
}

void BookContextMenu::addSeparator(QMenu* menu)
{
    QAction* newAction = new QAction(this);
    newAction->setSeparator(true);
    menu->addAction(newAction);
}
