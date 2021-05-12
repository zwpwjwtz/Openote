#ifndef BOOKACTIONDISPATCHER_H
#define BOOKACTIONDISPATCHER_H

#include "bookcontextmenu.h"


class BookView;

class BookActionDispatcher : public QObject
{
    Q_OBJECT
public:
    explicit BookActionDispatcher(QObject *parent = nullptr);

    void setView(BookView* view);
    void setMenu(BookContextMenu* menu);

private:
    BookView* view;
    BookContextMenu* menu;

private slots:
    void onContextMenuClicked(BookContextMenu::MenuType menu,
                              BookContextMenu::ActionType action);
};

#endif // BOOKACTIONDISPATCHER_H
