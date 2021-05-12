#include "bookactiondispatcher.h"
#include "bookview.h"


BookActionDispatcher::BookActionDispatcher(QObject *parent) : QObject(parent)
{
    view = nullptr;
    menu = nullptr;
}

void BookActionDispatcher::setView(BookView* view)
{
    this->view = view;
}

void BookActionDispatcher::setMenu(BookContextMenu* menu)
{
    this->menu = menu;

    if (menu != nullptr)
    {
        connect(menu,
                SIGNAL(itemClicked(BookContextMenu::MenuType,
                                   BookContextMenu::ActionType)),
                this,
                SLOT(onContextMenuClicked(BookContextMenu::MenuType,
                                          BookContextMenu::ActionType)));
    }

}

void
BookActionDispatcher::onContextMenuClicked(BookContextMenu::MenuType menu,
                                           BookContextMenu::ActionType action)
{
    if (view == nullptr)
        return;

    using MenuType = BookContextMenu::MenuType;
    using ActionType = BookContextMenu::ActionType;

    switch (menu)
    {
        case MenuType::TableMenu:
        switch (action)
        {
            case ActionType::Adding:
                view->addTable();    break;
            case ActionType::Deleting:
                view->deleteTable();    break;
            case ActionType::Duplicating:
                view->duplicateTable();    break;
            case ActionType::Renaming:
                view->renameTable();    break;
            default:;
        }  break;

        case MenuType::ColumnMenu:
        switch (action)
        {
            case ActionType::Adding:
                view->addColumn();    break;
            case ActionType::Deleting:
                view->deleteColumn();    break;
            case ActionType::Duplicating:
                view->duplicateColumn();    break;
            case ActionType::Renaming:
                view->renameColumn();    break;
            default:;
        }  break;

        case MenuType::RowMenu:
        switch (action)
        {
            case ActionType::Adding:
                view->addRow();    break;
            case ActionType::Deleting:
                view->deleteRow();    break;
            case ActionType::Duplicating:
                view->duplicateRow();    break;
            default:;
        }  break;

        case MenuType::GridMenu:
        switch (action)
        {
            case ActionType::AddingColumn:
                view->addColumn();    break;
            case ActionType::DeletingColumn:
                view->deleteColumn();    break;
            case ActionType::AddingRow:
                view->addRow();    break;
            case ActionType::DeletingRow:
                view->deleteRow();    break;
            default:;
        }  break;

        default:;
    }
}
