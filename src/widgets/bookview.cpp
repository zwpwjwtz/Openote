#include <QApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QMouseEvent>
#include "bookview.h"
#include "bookviewtabbar.h"
#include "bookview_p.h"
#include "tableview.h"
#include "columnreferencedelegate.h"
#include "bookcontextmenu.h"
#include "bookactiondispatcher.h"
#include "models/tablemodel.h"
#include "models/clipboardmodel.h"
#include "dialogs/dialogcolumnadd.h"
#include "dialogs/dialogfind.h"


BookView::BookView(QWidget *parent) : QTabWidget(parent)
{
    d_ptr = new BookViewPrivate(this);
    setTabBar(d_ptr->tabBar);
    
    setDocumentMode(true);
    setTabPosition(TabPosition::South);

    clear();
}

BookView::~BookView()
{
    delete d_ptr;
}

void BookView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton &&
        !event->flags().testFlag(Qt::MouseEventCreatedDoubleClick))
    {
        // Show context menu
        QWidget* widget = QApplication::widgetAt(mapToGlobal(event->pos()));
        if (widget == tabBar())
        {
            BookContextMenu* contextMenu = d_ptr->getContextMenu();
            contextMenu->setTableIsActive(
                        tabBar()->tabAt(event->pos() - tabBar()->pos()) >= 0);
            contextMenu->showTableMenu();
        }
    }
}

void BookView::clear()
{
    QTabWidget::clear();
    d_ptr->book.clear();
    d_ptr->book.setPath("");
    d_ptr->isModified = false;
}

bool BookView::loadBook(const QString& path)
{
    clear();

    d_ptr->book.setPath(path);
    if (!d_ptr->book.load())
        return false;

    TableModel* model;
    TableView* viewTable;
    for (int i=0; i<d_ptr->book.tableCount(); i++)
    {
        model = d_ptr->book.table(i);
        d_ptr->bindTableModel(model);

        viewTable = new TableView(this);
        d_ptr->bindTableView(viewTable, model);
        viewTable->scrollToBottom();
        addTab(viewTable, d_ptr->book.getTableName(i));
    }

    return true;
}

bool BookView::loadDefaultBook()
{
    clear();

    // Add an empty table
    QString newTableName(tr("Table1"));
    TableModel* newTable = d_ptr->book.addTable(newTableName);
    if (newTable == nullptr)
        return false;
    d_ptr->bindTableModel(newTable);
    newTable->newColumn(tr("New Column").toStdString(),
                        TableModel::ColumnType::String);
    newTable->newRow();

    TableView* viewTable = new TableView(this);
    d_ptr->bindTableView(viewTable, newTable);
    addTab(viewTable, newTableName);
    return true;
}

bool BookView::saveBook(const QString& path)
{
    d_ptr->book.setPath(path);
    if (d_ptr->book.save())
    {
        d_ptr->isModified = false;
        return true;
    }
    else
        return false;
}

QString BookView::currentPath() const
{
    return d_ptr->book.path();
}

void BookView::setPath(const QString &newPath)
{
    d_ptr->book.setPath(newPath);
}

bool BookView::modified() const
{
    return d_ptr->isModified;
}

bool BookView::selected() const
{
    auto index = d_ptr->getCurrentIndex();
    return index.table >= 0 && index.column >= 0 && index.row >= 0;
}

bool BookView::pastingEnabled() const
{
    return d_ptr->getClipboard()->canPaste();
}

bool BookView::addColumn()
{
    auto index = d_ptr->getCurrentIndex();
    if (index.table < 0)
        return false;

    DialogColumnAdd* dialog = d_ptr->getColumnAddDialog();
    if (dialog == nullptr)
    {
        dialog = new DialogColumnAdd(this);
        connect(dialog, SIGNAL(finished(int)),
                d_ptr, SLOT(onDialogColumnAddFinished(int)));
    }
    if (d_ptr->book.table(index.table)->countColumn() < 1)
    {
        dialog->enableReference = false;
        dialog->referring = false;
    }
    else
    {
        dialog->enableReference = true;
        dialog->referenceList.clear();
        for (int i=0; i<d_ptr->book.tableCount(); i++)
            dialog->referenceList.push_back(d_ptr->book.getTableName(i));
    }

    dialog->open();
    return true;
}

bool BookView::deleteColumn()
{
    auto index = d_ptr->getCurrentIndex();
    if (!index.isValid())
        return false;

    d_ptr->book.table(index.table)->removeColumns(index.column, 1);
    d_ptr->isModified = true;
        return true;
}

bool BookView::duplicateColumn()
{
    auto index = d_ptr->getCurrentIndex();
    if (index.table < 0 || index.column < 0)
        return false;

    QString oldName = d_ptr->columnHeader(index.table, index.column);
    QString newName = QInputDialog::getText(this, tr("Duplicate a column"),
                                            tr("New column name:"),
                                            QLineEdit::Normal,
                                            oldName);
    if (newName.isEmpty())
        return false;

    TableModel* table = d_ptr->book.table(index.table);
    if (table->duplicateColumn(index.column, newName.toStdString()))
    {

        d_ptr->isModified = true;
        return true;
    }
    else
        return false;
}

bool BookView::renameColumn()
{
    auto index = d_ptr->getCurrentIndex();
    if (!index.isValid())
        return false;

    QString oldName = d_ptr->columnHeader(index.table, index.column);
    QString newName = QInputDialog::getText(this, tr("Rename a column"),
                                            tr("New name for the column:"),
                                            QLineEdit::Normal,
                                            oldName);
    newName.replace("\n", "");
    newName.replace("\"", "'");
    if (newName.isEmpty() || newName == oldName)
        return true;

    if (d_ptr->setColumnHeader(newName, index.table, index.column))
    {
        d_ptr->isModified = true;
        return true;
    }
    else
        return false;
}

bool BookView::columnToTable()
{
    auto index = d_ptr->getCurrentIndex();
    if (index.table < 0 || index.column < 0)
        return false;

    TableModel* table = d_ptr->book.table(index.table);
    QString tableName =
            QInputDialog::getText(this, tr("Convert a column to table"),
                                  tr("New table name"),
                                  QLineEdit::Normal,
                                  QString::fromStdString(
                                              table->columnName(index.column)));
    if (tableName.isEmpty())
        return false;

    TableModel* newTable = d_ptr->book.convertColumnToTable(index.table,
                                                            index.column,
                                                            tableName);
    d_ptr->bindTableModel(newTable);

    TableView* viewTable = new TableView(this);
    d_ptr->bindTableView(viewTable, newTable);
    addTab(viewTable, tableName);
    setCurrentWidget(viewTable);

    d_ptr->isModified = true;
    return true;
}

bool BookView::addRow()
{
    auto index = d_ptr->getCurrentIndex();
    if (index.table < 0)
        return false;

    TableModel* table = d_ptr->book.table(index.table);
    if (table->countColumn() == 0)
    {
        QMessageBox::warning(this, tr("No column presents"),
                             tr("Please add a column before adding rows."));
        return false;
    }
    table->newRow();

    d_ptr->isModified = true;
    return true;
}

bool BookView::deleteRow()
{
    auto index = d_ptr->getCurrentIndex();
    if (!index.isValid())
        return false;

    TableModel* table = d_ptr->book.table(index.table);
    table->removeRows(index.row, 1);

    d_ptr->isModified = true;
    return true;
}

bool BookView::duplicateRow()
{
    auto index = d_ptr->getCurrentIndex();
    if (!index.isValid())
        return false;

    TableModel* table = d_ptr->book.table(index.table);
    if (table->duplicateRow(index.row))
    {
        d_ptr->isModified = true;
        return true;
    }
    else
        return false;
}

bool BookView::addTable()
{
    QString newName = QInputDialog::getText(this, tr("Add a table"),
                                            tr("Table name:"),
                                            QLineEdit::Normal,
                                            tr("NewTable"));
    if (newName.isEmpty())
        return false;

    TableModel* newTable = d_ptr->book.addTable(newName);
    d_ptr->bindTableModel(newTable);

    TableView* viewTable = new TableView(this);
    d_ptr->bindTableView(viewTable, newTable);
    addTab(viewTable, newName);
    setCurrentWidget(viewTable);

    d_ptr->isModified = true;
    return true;
}

bool BookView::deleteTable()
{
    auto index = d_ptr->getCurrentIndex();
    if (index.table < 0)
        return false;

    if (QMessageBox::warning(this, tr("Delete a table"),
                             QString(tr("Are you sure to delete table %1 ?"))
                                    .arg(d_ptr->book.getTableName(index.table)),
                             QMessageBox::Yes | QMessageBox::No)
            != QMessageBox::Yes)
        return false;

    if (d_ptr->book.removeTable(index.table))
    {
        removeTab(currentIndex());
        d_ptr->isModified = true;
        return true;
    }
    else
        return false;
}

bool BookView::duplicateTable()
{
    auto index = d_ptr->getCurrentIndex();
    if (index.table < 0)
        return false;

    QString oldName = d_ptr->book.getTableName(index.table);
    QString newName = QInputDialog::getText(this, tr("Duplicate a table"),
                                            tr("New table name:"),
                                            QLineEdit::Normal,
                                            QString(tr("%1_copy"))
                                                   .arg(oldName));
    if (newName.isEmpty())
        return false;

    // TODO: duplicate the table content
    TableModel* newTable = d_ptr->book.duplicateTable(index.table, newName);
    if (newTable == nullptr)
        return false;
    d_ptr->bindTableModel(newTable);

    TableView* viewTable = new TableView(this);
    d_ptr->bindTableView(viewTable, newTable);
    addTab(viewTable, newName);
    setCurrentWidget(viewTable);

    d_ptr->isModified = true;
    return true;
}

bool BookView::renameTable()
{
    auto index = d_ptr->getCurrentIndex();
    if (index.table < 0)
        return false;

    QString oldName = d_ptr->book.getTableName(index.table);
    QString newName = QInputDialog::getText(this, tr("Rename a table"),
                                            tr("New name for the table:"),
                                            QLineEdit::Normal,
                                            oldName);
    newName.replace("\n", "");
    newName.replace("\"", "'");
    if (newName.isEmpty() || newName == oldName)
        return true;

    if (d_ptr->book.setTableName(index.table, newName))
    {
        setTabText(currentIndex(), newName);
        d_ptr->isModified = true;
        return true;
    }
    return false;
}

bool BookView::copyContent()
{
    auto index = d_ptr->getCurrentIndex();
    TableModel* table = d_ptr->book.table(index.table);
    switch (table->columnType(index.column))
    {
        case TableModel::Integer:
            d_ptr->getClipboard()->copy(
                        table->readInt(index.row, index.column));
            break;
        case TableModel::Double:
            d_ptr->getClipboard()->copy(
                        table->readDouble(index.row, index.column));
            break;
        case TableModel::String:
            d_ptr->getClipboard()->copy(
                        table->readString(index.row, index.column));
            break;
        case TableModel::IntegerList:
            d_ptr->getClipboard()->copy(
                        table->readIntList(index.row, index.column));
            break;
        default:
            return false;
    }
    return true;
}

bool BookView::cutContent()
{
    auto index = d_ptr->getCurrentIndex();
    TableModel* table = d_ptr->book.table(index.table);
    switch (table->columnType(index.column))
    {
        case TableModel::Integer:
            d_ptr->getClipboard()->copy(
                        table->readInt(index.row, index.column));
            break;
        case TableModel::Double:
            d_ptr->getClipboard()->copy(
                        table->readDouble(index.row, index.column));
            break;
        case TableModel::String:
            d_ptr->getClipboard()->copy(
                        table->readString(index.row, index.column));
            break;
        case TableModel::IntegerList:
            d_ptr->getClipboard()->copy(
                        table->readIntList(index.row, index.column));
            break;
        default:
            return false;
    }
    table->ONTable::clear(index.row, index.column);
    return true;
}

bool BookView::pasteContent()
{
    auto index = d_ptr->getCurrentIndex();
    TableModel* table = d_ptr->book.table(index.table);
    switch (table->columnType(index.column))
    {
        case TableModel::Integer:
            return table->modify(index.row, index.column,
                                 d_ptr->getClipboard()->pasteAsInt());
        case TableModel::Double:
            return table->modify(index.row, index.column,
                                 d_ptr->getClipboard()->pasteAsDouble());
        case TableModel::String:
            return table->modify(index.row, index.column,
                                 d_ptr->getClipboard()->pasteAsString());
        case TableModel::IntegerList:
            return table->modify(index.row, index.column,
                                 d_ptr->getClipboard()->pasteAsIntList());
        default:
            return false;
    }
}

bool BookView::deleteContent()
{
    auto index = d_ptr->getCurrentIndex();
    if (!index.isValid())
        return false;

    TableModel* table = d_ptr->book.table(index.table);
    table->ONTable::clear(index.row, index.column);
    return true;
}

bool BookView::findContent()
{
    d_ptr->getFindDialog()->open();
    return true;
}

BookViewPrivate::BookViewPrivate(BookView* parent)
{
    q_ptr = parent;
    
    clipboard = nullptr;
    dialogColumnAdd = nullptr;
    dialogFind = nullptr;
    contextMenu = new BookContextMenu(q_ptr);
    actionDispatcher = new BookActionDispatcher(q_ptr);
    actionDispatcher->setView(q_ptr);
    actionDispatcher->setMenu(contextMenu);
    referenceDelegate = new ColumnReferenceDelegate();
    referenceDelegate->book = &book;
    tabBar = new BookViewTabbar(q_ptr);

    connect(q_ptr, SIGNAL(currentChanged(int)),
            this, SLOT(onTabCurrentIndexChanged(int)));
    connect(tabBar, SIGNAL(tabBarDoubleClicked(int)),
            this, SLOT(onTabBarDoubleClicked(int)));
}

BookViewPrivate::~BookViewPrivate()
{
    delete clipboard;
    delete contextMenu;
    delete actionDispatcher;
    delete dialogColumnAdd;
    delete dialogFind;
    delete referenceDelegate;
    delete tabBar;
}

BookIndex BookViewPrivate::getCurrentIndex()
{
    onTabCurrentIndexChanged(q_ptr->currentIndex());
    return lastIndex;
}

void BookViewPrivate::bindTableView(TableView* table, TableModel* model)
{
    table->setModel(model);
    table->setID(model->ID);
    table->setItemDelegate(referenceDelegate);

    connect(table, SIGNAL(columnHeaderRightClicked(int)),
            this, SLOT(onColumnHeaderRightClicked(int)));
    connect(table, SIGNAL(columnHeaderDoubleClicked(int)),
            this, SLOT(onColumnHeaderDoubleClicked(int)));
    connect(table, SIGNAL(gridRightClicked(int, int)),
            this, SLOT(onGridRightClicked(int, int)));
}

void BookViewPrivate::bindTableModel(const TableModel* model)
{
    connect(model, SIGNAL(dataChanged(const QModelIndex, const QModelIndex,
                                         const QVector<int>)),
            this, SLOT(onTableDataChanged()));
}

QString BookViewPrivate::columnHeader(int tableIndex, int columnIndex) const
{
    return book.table(tableIndex)->
                            headerData(columnIndex, Qt::Horizontal).toString();
}

bool BookViewPrivate::setColumnHeader(const QString &text,
                               int tableIndex, int columnIndex)
{
    book.table(tableIndex)->setHeaderData(columnIndex, Qt::Horizontal, text);
    return true;
}

void BookViewPrivate::findText(QString text, bool caseSensitive,
                               bool forward, bool inAllTables)
{
    BookIndex lastIndex = getCurrentIndex();
    if (!lastIndex.isValid())
    {
        if (lastIndex.table < 0)
            lastIndex.table = q_ptr->currentIndex();
        lastIndex.column = lastIndex.row = 0;
    }

    BookIndex foundIndex =
                book.find(text, lastIndex, forward, inAllTables, caseSensitive);
    if (foundIndex.isValid())
    {
        q_ptr->setCurrentIndex(foundIndex.table);
        if (lastIndex == foundIndex)
            QMessageBox::information(q_ptr, tr("One match found"),
                                     tr("The searched text was found "
                                     "in only one item."));
        else
        {
            dynamic_cast<TableView*>(q_ptr->widget(foundIndex.table))
                                          ->setCurrentIndex(foundIndex.row,
                                                            foundIndex.column);
            dialogFind->activateWindow();
        }
    }
    else
    {
        if (inAllTables)
            QMessageBox::information(q_ptr, tr("No matched item"),
                                     tr("The searched text was not found "
                                        "in any tables."));
        else
            QMessageBox::information(q_ptr, tr("No matched item"),
                                     tr("The searched text was not found "
                                        "in the current table."));
    }
}

ClipboardModel* BookViewPrivate::getClipboard()
{
    if (!clipboard)
        clipboard = new ClipboardModel();
    return clipboard;
}

BookContextMenu* BookViewPrivate::getContextMenu()
{
    return contextMenu;
}

DialogFind* BookViewPrivate::getFindDialog()
{
    if (dialogFind == nullptr)
    {
        dialogFind = new DialogFind(q_ptr);
        connect(dialogFind, SIGNAL(findNext(QString)),
                this, SLOT(onDialogFindStart(QString)));
        connect(dialogFind, SIGNAL(findPrevious(QString)),
                this, SLOT(onDialogFindStart(QString)));
    }
    return dialogFind;
}

DialogColumnAdd* BookViewPrivate::getColumnAddDialog()
{
    if (dialogColumnAdd == nullptr)
    {
        dialogColumnAdd = new DialogColumnAdd(q_ptr);
        connect(dialogColumnAdd, SIGNAL(finished(int)),
                this, SLOT(onDialogColumnAddFinished(int)));
    }
    return dialogColumnAdd;
}

void BookViewPrivate::onDialogColumnAddFinished(int result)
{
    if (result == QDialog::Rejected || dialogColumnAdd->newName.isEmpty())
        return;

    TableModel* table = book.table(lastIndex.table);
    TableModel* referredTable = nullptr;
    if (dialogColumnAdd->referring)
    {
         referredTable = book.table(dialogColumnAdd->referenceIndex);
        if (table == referredTable)
        {
            QMessageBox::warning(q_ptr, tr("Failed creating column"),
                                 tr("Creating a column referring to its "
                                 "parent table is not allowed. \n"
                                 "Please select another reference table."));
            dialogColumnAdd->open();
            return;
        }
    }

    std::string newName = dialogColumnAdd->newName.toStdString();
    if (dialogColumnAdd->referring)
    {
        if (table->newColumn(newName,
                             TableModel::ColumnType::IntegerList,
                             referredTable->ID) <= 0)
        {
            QMessageBox::critical(q_ptr, tr("Failed creating column"),
                                  tr("An error occurred when creating a column "
                                  "referring to another table."));
            return;
        }
    }
    else
    {
        switch (dialogColumnAdd->typeIndex)
        {
            case 0:  // Integer
                table->newColumn(newName, TableModel::ColumnType::Integer);
                break;
            case 1:  // Double
                table->newColumn(newName, TableModel::ColumnType::Double);
                break;
            case 2:  // String
                table->newColumn(newName, TableModel::ColumnType::String);
                break;
            default:;
        }
    }

    isModified = true;
}

void BookViewPrivate::onDialogFindStart(QString text)
{
    findText(text, dialogFind->caseSensitive(),
             !dialogFind->backward(), dialogFind->findInAllTables());
}

void BookViewPrivate::onTableDataChanged()
{
    isModified = true;
}

void BookViewPrivate::onColumnHeaderRightClicked(int index)
{
    contextMenu->setColumnIsActive(index >= 0);
    contextMenu->showColumnMenu();
}

void BookViewPrivate::onColumnHeaderDoubleClicked(int index)
{
    if (index >= 0)
        q_ptr->renameColumn();
    else
        q_ptr->addColumn();
}

void BookViewPrivate::onTabBarDoubleClicked(int index)
{
    if (index >= 0)
        q_ptr->renameTable();
    else
        q_ptr->addTable();
}

void BookViewPrivate::onTabCurrentIndexChanged(int tableIndex)
{
    TableView* table = dynamic_cast<TableView*>(q_ptr->widget(tableIndex));
    if (table == nullptr)
    {
        lastIndex.table = -1;
        return;
    }
    lastIndex.table = tableIndex;

    // TODO: deal with tables of empty row but of non-empty columns
    QModelIndex modelIndex = table->currentIndex();
    lastIndex.column = modelIndex.column();
    lastIndex.row = modelIndex.row();
}

void BookViewPrivate::onGridRightClicked(int rowIndex, int columnIndex)
{
    contextMenu->setGridIsActive(rowIndex >= 0 && columnIndex >= 0);
    contextMenu->showGridMenu();
}
