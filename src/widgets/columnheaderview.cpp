#include <QMouseEvent>
#include "columnheaderview.h"


ColumnHeaderView::ColumnHeaderView(QWidget* parent) :
    QHeaderView (Qt::Horizontal, parent)
{}

void ColumnHeaderView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
        emit rightClicked(logicalIndexAt(event->pos()));
    else
        QHeaderView::mousePressEvent(event);
}

void ColumnHeaderView::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        emit leftDoubleClicked(logicalIndexAt(event->pos()));
    else
        ColumnHeaderView::mousePressEvent(event);
}
