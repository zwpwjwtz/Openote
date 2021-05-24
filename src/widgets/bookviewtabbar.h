#ifndef BOOKVIEWTABBAR_H
#define BOOKVIEWTABBAR_H

#include <QTabBar>


class BookViewTabbar : public QTabBar
{
public:
    explicit BookViewTabbar(QWidget* parent = nullptr);

protected:
    void wheelEvent(QWheelEvent* event);
};

#endif // BOOKVIEWTABBAR_H
