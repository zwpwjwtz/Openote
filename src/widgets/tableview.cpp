#include <QMouseEvent>
#include <QHeaderView>
#include "tableview.h"
#include "columnheaderview.h"


TableView::TableView(QWidget* parent) :
    QTableView (parent)
{
    tableID = 0;

    columnHeader = new ColumnHeaderView(this);
    connect(columnHeader, SIGNAL(rightClicked(int)),
            this, SIGNAL(columnHeaderRightClicked(int)));
    connect(columnHeader, SIGNAL(leftDoubleClicked(int)),
            this, SIGNAL(columnHeaderDoubleClicked(int)));
    setHorizontalHeader(columnHeader);
}

void TableView::keyPressEvent(QKeyEvent* event)
{
    // Response to key events only when:
    // ... user is editing a cell
    // ... user is trying to edit a cell
    // ... or user is pressing one of the navigation keys
    if (state() == State::EditingState)
        QTableView::keyPressEvent(event);
    else
    switch (event->key())
    {
        case Qt::Key_Enter:
        case Qt::Key_Space:
        case Qt::Key_Escape:
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Home:
        case Qt::Key_End:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
        case Qt::Key_Tab:
            QTableView::keyPressEvent(event);
        default:;
    }
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

void TableView::setCurrentIndex(int row, int column)
{
    QTableView::setCurrentIndex(model()->index(row, column));
}
