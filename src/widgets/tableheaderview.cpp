#include <QMouseEvent>
#include "tableheaderview.h"


TableHeaderView::TableHeaderView(QWidget* parent, Qt::Orientation orientation)
    : QHeaderView (orientation, parent)
{
    setSectionsClickable(true);
}

void TableHeaderView::mousePressEvent(QMouseEvent* event)
{
    QHeaderView::mousePressEvent(event);
    if (event->button() == Qt::RightButton)
    {
        int index = logicalIndexAt(event->pos());
        emit sectionPressed(index);
        emit rightClicked(index);
    }
}

void TableHeaderView::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        emit leftDoubleClicked(logicalIndexAt(event->pos()));
    else
        TableHeaderView::mousePressEvent(event);
}
