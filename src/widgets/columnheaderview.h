#ifndef COLUMNHEADERVIEW_H
#define COLUMNHEADERVIEW_H

#include <QHeaderView>


class ColumnHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    explicit ColumnHeaderView(QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);

signals:
    void rightClicked(int index);
    void leftDoubleClicked(int index);
};

#endif // COLUMNHEADERVIEW_H
