#include <QApplication>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QMouseEvent>
#include "bookview.h"
#include "tableview.h"
#include "columnreferencedelegate.h"
#include "bookcontextmenu.h"
#include "bookactiondispatcher.h"
#include "models/tablemodel.h"
#include "models/clipboardmodel.h"
#include "dialogs/dialogcolumnadd.h"


BookView::BookView(QWidget *parent) : QTabWidget(parent)
{
    clipboard = new ClipboardModel;
    dialogColumnAdd = nullptr;
    referenceDelegate = new ColumnReferenceDelegate();
    referenceDelegate->book = &book;
    contextMenu = new BookContextMenu(this);
    actionDispatcher = new BookActionDispatcher(this);
    actionDispatcher->setView(this);
    actionDispatcher->setMenu(contextMenu);

    connect(tabBar(), SIGNAL(tabBarDoubleClicked(int)),
            this, SLOT(onTabBarDoubleClicked(int)));

    setDocumentMode(true);
    setTabPosition(TabPosition::South);

    clear();
}


BookView::~BookView()
{
    delete clipboard;
    delete dialogColumnAdd;
    delete referenceDelegate;
    delete contextMenu;
    delete actionDispatcher;
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
            contextMenu->setTableIsActive(
                        tabBar()->tabAt(event->pos() - tabBar()->pos()) >= 0);
            contextMenu->showTableMenu();
        }
    }
}

void BookView::clear()
{
    QTabWidget::clear();
    book.clear();
    book.setPath("");
    isModified = false;
}

bool BookView::loadBook(const QString& path)
{
    clear();

    book.setPath(path);
    if (!book.load())
        return false;

    auto tableIDs = book.tableIDs();
    for (auto i=tableIDs.cbegin(); i!=tableIDs.cend(); i++)
    {
        // Bind the model with a view
        TableView* viewTable = new TableView(this);
        viewTable->setModel(book.table(*i));
        bindTableView(viewTable);
        addTab(viewTable, book.tableName(*i));
    }

    return true;
}

bool BookView::loadDefaultBook()
{
    clear();

    // Add an empty table
    QString newTableName(tr("Table1"));
    TableModel* newTable = book.addTable(newTableName);
    if (newTable == nullptr)
        return false;
    bindTableModel(newTable);
    newTable->newColumn(tr("New Column").toStdString(),
                        TableModel::ColumnType::String);
    newTable->newRow();

    TableView* viewTable = new TableView(this);
    viewTable->setModel(newTable);
    bindTableView(viewTable);
    addTab(viewTable, newTableName);
    return true;
}

bool BookView::saveBook(const QString& path)
{
    book.setPath(path);
    if (book.save())
    {
        isModified = false;
        return true;
    }
    else
        return false;
}

QString BookView::currentPath() const
{
    return book.path();
}

void BookView::setPath(const QString &newPath)
{
    book.setPath(newPath);
}

bool BookView::modified() const
{
    return isModified;
}

bool BookView::selected() const
{
    BookIndex index = getCurrentIndex();
    return index.table >= 0 && index.column >= 0 && index.row >= 0;
}

bool BookView::pastingEnabled() const
{
    return clipboard->canPaste();
}

bool BookView::addColumn()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (index.table < 0)
        return false;

    if (dialogColumnAdd == nullptr)
    {
        dialogColumnAdd = new DialogColumnAdd(this);
        connect(dialogColumnAdd, SIGNAL(finished(int)),
                this, SLOT(onDialogColumnAddFinished(int)));
    }
    if (book.table(getTableID(index.table))->countColumn() < 1)
    {
        dialogColumnAdd->enableReference = false;
        dialogColumnAdd->referring = false;
    }
    else
    {
        dialogColumnAdd->enableReference = true;
        dialogColumnAdd->referenceList.clear();
        auto tableIDs = book.tableIDs();
        for (auto i=tableIDs.cbegin(); i!=tableIDs.cend(); i++)
            dialogColumnAdd->referenceList.push_back(book.tableName(*i));
    }

    dialogColumnAdd->open();
    return true;
}

bool BookView::deleteColumn()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (!index.isValid())
        return false;

    book.table(getTableID(index.table))->removeColumns(index.column,1 );
    isModified = true;
        return true;
}

bool BookView::duplicateColumn()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (index.table < 0 || index.column < 0)
        return false;

    QString oldName = columnHeader(index.table, index.column);
    QString newName = QInputDialog::getText(this, tr("Duplicate a column"),
                                            tr("New column name:"),
                                            QLineEdit::Normal,
                                            oldName);
    if (newName.isEmpty())
        return false;

    TableModel* table = book.table(getTableID(index.table));
    if (table->duplicateColumn(index.column, newName))
    {

        isModified = true;
        return true;
    }
    else
        return false;
}

bool BookView::renameColumn()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (!index.isValid())
        return false;

    QString oldName = columnHeader(index.table, index.column);
    QString newName = QInputDialog::getText(this, tr("Rename a column"),
                                            tr("New name for the column:"),
                                            QLineEdit::Normal,
                                            oldName);
    newName.replace("\n", "");
    newName.replace("\"", "'");
    if (newName.isEmpty() || newName == oldName)
        return true;

    if (setColumnHeader(newName, index.table, index.column))
    {
        isModified = true;
        return true;
    }
    else
        return false;
}

bool BookView::columnToTable()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (index.table < 0 || index.column < 0)
        return false;

    TableModel* table = book.table(getTableID(index.table));
    int columnID = table->columnID(index.column);
    QString tableName =
            QInputDialog::getText(this, tr("Convert a column to table"),
                                  tr("New table name"),
                                  QLineEdit::Normal,
                                  QString::fromStdString(
                                                  table->columnName(columnID)));
    if (tableName.isEmpty())
        return false;

    TableModel* newTable =
                    book.convertColumnToTable(table, columnID, tableName);
    bindTableModel(newTable);

    TableView* viewTable = new TableView(this);
    viewTable->setModel(newTable);
    bindTableView(viewTable);
    addTab(viewTable, tableName);
    setCurrentWidget(viewTable);

    isModified = true;
    return true;
}

bool BookView::addRow()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (index.table < 0)
        return false;

    TableModel* table = book.table(getTableID(index.table));
    if (table->countColumn() == 0)
    {
        QMessageBox::warning(this, tr("No column presents"),
                             tr("Please add a column before adding rows."));
        return false;
    }
    table->newRow();

    isModified = true;
    return true;
}

bool BookView::deleteRow()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (!index.isValid())
        return false;

    TableModel* table = book.table(getTableID(index.table));
    table->removeRows(index.row, 1);

    isModified = true;
    return true;
}

bool BookView::duplicateRow()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (!index.isValid())
        return false;

    TableModel* table = book.table(getTableID(index.table));
    if (table->duplicateRow(index.row))
    {
        isModified = true;
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

    TableModel* newTable = book.addTable(newName);
    bindTableModel(newTable);

    TableView* viewTable = new TableView(this);
    viewTable->setModel(newTable);
    bindTableView(viewTable);
    addTab(viewTable, newName);
    setCurrentWidget(viewTable);

    isModified = true;
    return true;
}

bool BookView::deleteTable()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (index.table < 0)
        return false;

    int tableID = getTableID(index.table);
    if (QMessageBox::warning(this, tr("Delete a table"),
                             QString(tr("Are you sure to delete table %1 ?"))
                                    .arg(book.tableName(tableID)),
                             QMessageBox::Yes | QMessageBox::No)
            != QMessageBox::Yes)
        return false;

    if (book.removeTable(getTableID(index.table)))
    {
        removeTab(currentIndex());
        isModified = true;
        return true;
    }
    else
        return false;
}

bool BookView::duplicateTable()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (index.table < 0)
        return false;

    int oldTableID = getTableID(index.table);
    QString oldName = book.tableName(oldTableID);
    QString newName = QInputDialog::getText(this, tr("Duplicate a table"),
                                            tr("New table name:"),
                                            QLineEdit::Normal,
                                            QString(oldName).append("Copy"));
    if (newName.isEmpty())
        return false;

    // TODO: duplicate the table content
    TableModel* newTable = book.duplicateTable(oldTableID, newName);
    if (newTable == nullptr)
        return false;
    bindTableModel(newTable);

    TableView* viewTable = new TableView(this);
    viewTable->setModel(newTable);
    bindTableView(viewTable);
    addTab(viewTable, newName);
    setCurrentWidget(viewTable);

    isModified = true;
    return true;
}

bool BookView::renameTable()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (index.table < 0)
        return false;

    int tableID = getTableID(index.table);
    QString oldName = book.tableName(tableID);
    QString newName = QInputDialog::getText(this, tr("Rename a table"),
                                            tr("New name for the table:"),
                                            QLineEdit::Normal,
                                            oldName);
    newName.replace("\n", "");
    newName.replace("\"", "'");
    if (newName.isEmpty() || newName == oldName)
        return true;

    if (book.setTableName(tableID, newName))
    {
        setTabText(currentIndex(), newName);
        isModified = true;
        return true;
    }
    return false;
}

bool BookView::copyContent()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    TableModel* table = book.table(getTableID(index.table));
    int columnID = table->columnID(index.column);
    int rowID = table->rowID(index.row);
    switch (table->columnType(columnID))
    {
        case TableModel::Integer:
            clipboard->copy(table->readInt(rowID, columnID));
            break;
        case TableModel::Double:
            clipboard->copy(table->readDouble(rowID, columnID));
            break;
        case TableModel::String:
            clipboard->copy(table->readString(rowID, columnID));
            break;
        case TableModel::IntegerList:
            clipboard->copy(table->readIntList(rowID, columnID));
            break;
        default:
            return false;
    }
    return true;
}

bool BookView::cutContent()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    TableModel* table = book.table(getTableID(index.table));
    int columnID = table->columnID(index.column);
    int rowID = table->rowID(index.row);
    switch (table->columnType(columnID))
    {
        case TableModel::Integer:
            clipboard->copy(table->readInt(rowID, columnID));
            break;
        case TableModel::Double:
            clipboard->copy(table->readDouble(rowID, columnID));
            break;
        case TableModel::String:
            clipboard->copy(table->readString(rowID, columnID));
            break;
        case TableModel::IntegerList:
            clipboard->copy(table->readIntList(rowID, columnID));
            break;
        default:
            return false;
    }
    table->ONTable::clear(rowID, columnID);
    return true;
}

bool BookView::pasteContent()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    TableModel* table = book.table(getTableID(index.table));
    int columnID = table->columnID(index.column);
    int rowID = table->rowID(index.row);
    switch (table->columnType(columnID))
    {
        case TableModel::Integer:
            return table->modify(rowID, columnID, clipboard->pasteAsInt());
        case TableModel::Double:
            return table->modify(rowID, columnID, clipboard->pasteAsDouble());
        case TableModel::String:
            return table->modify(rowID, columnID, clipboard->pasteAsString());
        case TableModel::IntegerList:
            return table->modify(rowID, columnID, clipboard->pasteAsIntList());
        default:
            return false;
    }
}

bool BookView::deleteContent()
{
    BookIndex index = currentBookIndex = getCurrentIndex();
    if (!index.isValid())
        return false;

    TableModel* table = book.table(getTableID(index.table));
    int columnID = table->columnID(index.column);
    int rowID = table->rowID(index.row);
    table->ONTable::clear(rowID, columnID);
    return true;
}

int BookView::getTableID(int tableIndex) const
{
    TableView* table = dynamic_cast<TableView*>(widget(tableIndex));
    if (table == nullptr)
        return 0;
    else
        return table->ID();
}

int BookView::getTableIndex(int tableID) const
{
    auto tableIDList = book.tableIDs();
    auto index = std::find(tableIDList.cbegin(), tableIDList.cend(), tableID);
    if (index == tableIDList.cend())
        return -1;
    else
        return int(index - tableIDList.cbegin());
}

BookView::BookIndex BookView::getCurrentIndex() const
{
    BookIndex bookIndex{-1, -1, -1};

    if (!currentWidget())
        return bookIndex;
    bookIndex.table = getTableIndex(getTableID(currentIndex()));

    // TODO: deal with tables of empty row but of non-empty columns
    QModelIndex modelIndex =
                    dynamic_cast<TableView*>(currentWidget())->currentIndex();
    if (!modelIndex.isValid())
        return bookIndex;

    bookIndex.column = modelIndex.column();
    bookIndex.row = modelIndex.row();
    return bookIndex;
}

void BookView::bindTableView(TableView* table)
{
    TableModel* model = dynamic_cast<TableModel*>(table->model());
    if (!model)
        return;

    table->setID(model->ID);
    table->setItemDelegate(referenceDelegate);

    connect(table, SIGNAL(columnHeaderRightClicked(int)),
            this, SLOT(onColumnHeaderRightClicked(int)));
    connect(table, SIGNAL(columnHeaderDoubleClicked(int)),
            this, SLOT(onColumnHeaderDoubleClicked(int)));
    connect(table, SIGNAL(gridRightClicked(int, int)),
            this, SLOT(onGridRightClicked(int, int)));
}

void BookView::bindTableModel(const TableModel* model)
{
    connect(model, SIGNAL(dataChanged(const QModelIndex, const QModelIndex,
                                         const QVector<int>)),
            this, SLOT(onTableDataChanged()));
}

QString BookView::columnHeader(int tableIndex, int columnIndex) const
{
    return book.table(getTableID(tableIndex))->
                            headerData(columnIndex, Qt::Horizontal).toString();
}

bool BookView::setColumnHeader(const QString &text,
                               int tableIndex, int columnIndex)
{
    book.table(getTableID(tableIndex))->
                            setHeaderData(columnIndex, Qt::Horizontal, text);
    return true;
}

void BookView::onDialogColumnAddFinished(int result)
{
    if (result == QDialog::Rejected || dialogColumnAdd->newName.isEmpty())
        return;

    int tableID = getTableID(currentBookIndex.table);
    int referenceTableID;
    if (dialogColumnAdd->referring)
    {
        auto tableIDs = book.tableIDs();
        referenceTableID = tableIDs[dialogColumnAdd->referenceIndex];
        if (referenceTableID == tableID)
        {
            QMessageBox::warning(this, tr("Failed creating column"),
                                 tr("Creating a column referring to its "
                                 "parent table is not allowed. \n"
                                 "Please select another reference table."));
            dialogColumnAdd->open();
            return;
        }
    }

    TableModel* table = book.table(tableID);
    std::string newName = dialogColumnAdd->newName.toStdString();
    if (dialogColumnAdd->referring)
    {
        if (table->newColumn(newName,
                             TableModel::ColumnType::IntegerList,
                             referenceTableID) <= 0)
        {
            QMessageBox::critical(this, tr("Failed creating column"),
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

void BookView::onTableDataChanged()
{
    isModified = true;
}

void BookView::onColumnHeaderRightClicked(int index)
{
    contextMenu->setColumnIsActive(index >= 0);
    contextMenu->showColumnMenu();
}

void BookView::onColumnHeaderDoubleClicked(int index)
{
    if (index >= 0)
        renameColumn();
    else
        addColumn();
}

void BookView::onTabBarDoubleClicked(int index)
{
    if (index >= 0)
        renameTable();
    else
        addTable();
}

void BookView::onGridRightClicked(int rowIndex, int columnIndex)
{
    contextMenu->setGridIsActive(rowIndex >= 0 && columnIndex >= 0);
    contextMenu->showGridMenu();
}
