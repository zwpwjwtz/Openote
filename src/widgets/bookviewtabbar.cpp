#include <QWheelEvent>
#include "bookviewtabbar.h"


BookViewTabbar::BookViewTabbar(QWidget* parent):
    QTabBar (parent)
{}

void BookViewTabbar::wheelEvent(QWheelEvent* event)
{
    event->ignore();
}
