#ifndef COLUMNREFERENCEDELEGATE_H
#define COLUMNREFERENCEDELEGATE_H

#include <QItemDelegate>


class BookModel;
class QStandardItemModel;

class ColumnReferenceDelegate : public QItemDelegate
{
public:
    BookModel* book;

    ColumnReferenceDelegate();

    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const;

    virtual QWidget* createEditor(QWidget *parent,
                                  const QStyleOptionViewItem& option,
                                  const QModelIndex& index) const;

    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const;
    virtual void setModelData(QWidget* editor,
                              QAbstractItemModel* model,
                              const QModelIndex& index) const;

private:
    QStandardItemModel* listModel;
};

#endif // COLUMNREFERENCEDELEGATE_H
