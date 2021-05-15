#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>


class ColumnHeaderView;

class TableView : public QTableView
{
    Q_OBJECT

public:
    explicit TableView(QWidget *parent = nullptr);

    int ID() const;
    void setID(int ID);

protected:
    void mousePressEvent(QMouseEvent* event);

signals:
    void columnHeaderRightClicked(int columnIndex);
    void columnHeaderDoubleClicked(int columnIndex);
    void gridRightClicked(int rowIndex, int columnIndex);

private:
    int tableID;
    ColumnHeaderView* columnHeader;
};

#endif // TABLEVIEW_H
