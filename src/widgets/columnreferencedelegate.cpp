#include "columnreferencedelegate.h"
#include "models/bookmodel.h"
#include "models/tablemodel.h"
#include <QStandardItemModel>
#include <QListView>

#define OPENOTE_DELEGATE_EDITOR_PROP_ID    "column-reference"
#define OPENOTE_DELEGATE_EDITOR_HEIGHT     6

ColumnReferenceDelegate::ColumnReferenceDelegate()
{
    listModel = new QStandardItemModel(this);
}

QSize ColumnReferenceDelegate::sizeHint(const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const
{
    QSize newSize(option.decorationSize);
    newSize.setHeight(newSize.height() * OPENOTE_DELEGATE_EDITOR_HEIGHT);
    return newSize;
}

QWidget*
ColumnReferenceDelegate::createEditor(QWidget* parent,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const
{
    const TableModel* table = dynamic_cast<const TableModel*>(index.model());
    if (table == nullptr || book == nullptr)
    {
        // The delegated object is not a TableModel
        // Fall back to the default editor
        return QItemDelegate::createEditor(parent, option, index);
    }

    const TableModel* referenceTable =
                    book->columnReferenceTable(table->ID,
                                               table->columnID(index.column()));
    if (referenceTable == nullptr)
    {
        // This column does not have any reference
        // Fall back to the default editor
        return QItemDelegate::createEditor(parent, option, index);
    }

    if (referenceTable->countColumn() < 1)
    {
        // No column in the reference table
        return nullptr;
    }

    // Load all data in the referred column into a list
    int referredColumnID = referenceTable->columnID(0);
    auto columnType = referenceTable->columnType(referredColumnID);
    auto rowIDs = referenceTable->IDs();
    QStandardItem* listItem;
    listModel->clear();
    for (auto i=rowIDs.cbegin(); i!=rowIDs.cend(); i++)
    {
        listItem = new QStandardItem();
        switch (columnType)
        {
            case TableModel::ColumnType::Integer:
                listItem->setText(QString::number(
                            referenceTable->readInt(*i, referredColumnID)));
                break;
            case TableModel::ColumnType::Double:
                listItem->setText(QString::number(
                            referenceTable->readDouble(*i, referredColumnID)));
                break;
            case TableModel::ColumnType::String:
                listItem->setText(
                    QString::fromStdString(
                            referenceTable->readString(*i, referredColumnID)));
                break;
            case TableModel::ColumnType::IntegerList:
            {
                QString displayedString;
                auto values = referenceTable->readIntList(*i, referredColumnID);
                for (auto j=values.cbegin(); j!=values.cend(); j++)
                    displayedString.append(*j);
                listItem->setText(displayedString);
                break;
            }
            default:;
        }
        listItem->setData(*i);
        listItem->setCheckable(true);
        listItem->setEditable(false);
        listModel->appendRow(listItem);
    }

    QListView* listEditor = new QListView(parent);
    listEditor->setProperty(OPENOTE_DELEGATE_EDITOR_PROP_ID, true);
    listEditor->setModel(listModel);
    listEditor->setModelColumn(0);
    return listEditor;
}

void ColumnReferenceDelegate::setEditorData(QWidget* editor,
                                            const QModelIndex& index) const
{
    QListView* listEditor = dynamic_cast<QListView*>(editor);
    if (listEditor == nullptr ||
        !listEditor->property(OPENOTE_DELEGATE_EDITOR_PROP_ID).isValid())
    {
        // Not an editor create by this class; ignore it
        return QItemDelegate::setEditorData(editor, index);
    }

    auto currentData = index.data(Qt::EditRole).toList();

    for (int i=0; i<currentData.count(); i++)
    {
        if (i >= listModel->rowCount())
            break;
        listModel->item(i, 0)->setCheckState(Qt::Checked);
    }
    return;
}

void ColumnReferenceDelegate::setModelData(QWidget* editor,
                                           QAbstractItemModel* model,
                                           const QModelIndex& index) const
{
    QListView* listEditor = dynamic_cast<QListView*>(editor);
    if (listEditor == nullptr ||
        !listEditor->property(OPENOTE_DELEGATE_EDITOR_PROP_ID).isValid())
    {
        // Not an editor create by this class; ignore it
        return QItemDelegate::setModelData(editor, model, index);
    }

    TableModel* table = dynamic_cast<TableModel*>(model);
    if (table == nullptr)
    {
        // The delegated object is not a TableModel
        return QItemDelegate::setModelData(editor, model, index);;
    }

    // Collect IDs of selected rows
    int rowID;
    QList<QVariant> selectedIDs;
    for (int i=0; i<listModel->rowCount(); i++)
    {
        if (listModel->item(i, 0)->checkState() != Qt::Checked)
            continue;

        rowID = listModel->item(i, 0)->data().toInt();
        if (rowID > 0)
            selectedIDs.push_back(rowID);
    }
    if (selectedIDs.size() < 1)
        return;

    // Update the model
    table->setData(index, selectedIDs);
    return;
}
