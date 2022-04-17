#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>


class TableHeaderView;

class TableView : public QTableView
{
    Q_OBJECT

public:
    explicit TableView(QWidget *parent = nullptr);

    int ID() const;
    void setID(int ID);

    void setCurrentIndex(int row, int column);

protected:
    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);

signals:
    void rowHeaderRightClicked(int rowIndex);
    void columnHeaderRightClicked(int columnIndex);
    void columnHeaderDoubleClicked(int columnIndex);
    void gridRightClicked(int rowIndex, int columnIndex);

private:
    int tableID;
    TableHeaderView* rowHeader;
    TableHeaderView* columnHeader;
};

#endif // TABLEVIEW_H
