#ifndef TABLEHEADERVIEW_H
#define TABLEHEADERVIEW_H

#include <QHeaderView>


class TableHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    explicit TableHeaderView(QWidget* parent = nullptr,
                             Qt::Orientation orientation = Qt::Horizontal);

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);

signals:
    void rightClicked(int index);
    void leftDoubleClicked(int index);
};

#endif // TABLEHEADERVIEW_H
