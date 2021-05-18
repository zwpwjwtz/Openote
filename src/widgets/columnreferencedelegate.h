#ifndef COLUMNREFERENCEDELEGATE_H
#define COLUMNREFERENCEDELEGATE_H

#include <QItemDelegate>


class BookModel;
class ColumnReferenceSelector;

class ColumnReferenceDelegate : public QItemDelegate
{
public:
    BookModel* book;

    ColumnReferenceDelegate();

    virtual QWidget* createEditor(QWidget *parent,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const;

    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
    virtual void setModelData(QWidget* editor,
                              QAbstractItemModel* model,
                              const QModelIndex& index) const;
    virtual void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const;


private slots:
    void onEditorAddingItemRequested(ColumnReferenceSelector* editor,
                                     QString text);
};

#endif // COLUMNREFERENCEDELEGATE_H
