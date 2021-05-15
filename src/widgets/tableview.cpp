#include <QMouseEvent>
#include <QHeaderView>
#include "tableview.h"
#include "columnheaderview.h"


TableView::TableView(QWidget* parent) :
    QTableView (parent)
{
    columnHeader = new ColumnHeaderView(this);
    connect(columnHeader, SIGNAL(rightClicked(int)),
            this, SIGNAL(columnHeaderRightClicked(int)));
    connect(columnHeader, SIGNAL(leftDoubleClicked(int)),
            this, SIGNAL(columnHeaderDoubleClicked(int)));
    setHorizontalHeader(columnHeader);
}

void TableView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::RightButton)
        return QTableView::mousePressEvent(event);

    emit gridRightClicked(this->rowAt(event->y()),
                          this->columnAt(event->x()));
}

int TableView::ID() const
{
    return tableID;
}

void TableView::setID(int ID)
{
    if (ID > 0)
        tableID = ID;
    else
        tableID = 0;
}
