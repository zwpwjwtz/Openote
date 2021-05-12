#ifndef COLUMNREFERENCESELECTOR_H
#define COLUMNREFERENCESELECTOR_H

#include <QFrame>
#include <QStandardItemModel>


class QStandardItemModel;

namespace Ui {
class ColumnReferenceSelector;
}

class ColumnReferenceSelector : public QFrame
{
    Q_OBJECT

public:
    explicit ColumnReferenceSelector(QWidget *parent = nullptr);

    int count() const;

    void clear();
    void addItem(int ID, int intValue);
    void addItem(int ID, double doubleValue);
    void addItem(int ID, const QString& text);

    QList<int> checkedIDs() const;
    void setChecked(int ID, bool checked = true);

    void setOptimizedSize(int visibleCount = 5);

protected:
    void resizeEvent(QResizeEvent* event);

signals:
    void addingItemRequested(ColumnReferenceSelector* editor,
                             QString text);

private:
    Ui::ColumnReferenceSelector* ui;
    QStandardItemModel listModel;
    QList<int> IDList;

private slots:
    void on_textSearch_textChanged();
    void on_buttonAdd_clicked();
};

#endif // COLUMNREFERENCESELECTOR_H
