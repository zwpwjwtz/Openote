#ifndef BOOKVIEW_H
#define BOOKVIEW_H

#include <QTabWidget>


class BookViewPrivate;

class BookView : public QTabWidget
{
    Q_OBJECT
public:
    explicit BookView(QWidget *parent = nullptr);
    ~BookView();

    void clear();
    bool loadBook(const QString& path);
    bool loadDefaultBook();
    bool saveBook(const QString& path = "");

    QString currentPath() const;
    void setPath(const QString& newPath);

    bool modified() const;
    bool selected() const;
    bool pastingEnabled() const;

public slots:
    bool addColumn();
    bool deleteColumn();
    bool duplicateColumn();
    bool renameColumn();
    bool columnToTable();

    bool addRow();
    bool deleteRow();
    bool duplicateRow();

    bool addTable();
    bool deleteTable();
    bool duplicateTable();
    bool renameTable();

    bool copyContent();
    bool cutContent();
    bool pasteContent();
    bool deleteContent();
    bool findContent();

protected:
    BookViewPrivate* d_ptr;
    void mousePressEvent(QMouseEvent* event);
};

#endif // BOOKVIEW_H
