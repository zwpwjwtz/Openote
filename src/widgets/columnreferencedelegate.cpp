#include "columnreferencedelegate.h"
#include "models/bookmodel.h"
#include "models/tablemodel.h"
#include "columnreferenceselector.h"
#include <QListView>
#include <QMessageBox>

#define OPENOTE_DELEGATE_EDITOR_PROP_TABLE "table-id"
#define OPENOTE_DELEGATE_EDITOR_PROP_COL   "column-id"


ColumnReferenceDelegate::ColumnReferenceDelegate(QObject* parent) :
    QItemDelegate (parent)
{}

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
                        book->columnReferenceTable(table->ID, index.column());
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
    int referredColumn = 0;
    auto columnType = referenceTable->columnType(referredColumn);
    auto rowIDs = referenceTable->IDs();

    ColumnReferenceSelector* listEditor = new ColumnReferenceSelector(parent);
    listEditor->setProperty(OPENOTE_DELEGATE_EDITOR_PROP_TABLE, table->ID);
    listEditor->setProperty(OPENOTE_DELEGATE_EDITOR_PROP_COL, index.column());
    connect(listEditor, &ColumnReferenceSelector::addingItemRequested,
            this, &ColumnReferenceDelegate::onEditorAddingItemRequested);
    for (auto i=rowIDs.cbegin(); i!=rowIDs.cend(); i++)
    {
        switch (columnType)
        {
            case TableModel::ColumnType::Integer:
                listEditor->addItem(*i,
                            referenceTable->readInt(*i, referredColumn));
                break;
            case TableModel::ColumnType::Double:
                listEditor->addItem(*i,
                            referenceTable->readDouble(*i, referredColumn));
                break;
            case TableModel::ColumnType::String:
                listEditor->addItem(*i,
                    QString::fromStdString(
                            referenceTable->readString(*i, referredColumn)));
                break;
            case TableModel::ColumnType::IntegerList:
            {
                QString displayedString;
                auto values = referenceTable->readIntList(*i, referredColumn);
                for (auto j=values.cbegin(); j!=values.cend(); j++)
                    displayedString.append(*j);
                listEditor->addItem(*i, displayedString);
                break;
            }
            default:;
        }
    }

    return listEditor;
}

void ColumnReferenceDelegate::setEditorData(QWidget* editor,
                                            const QModelIndex& index) const
{
    auto listEditor = dynamic_cast<ColumnReferenceSelector*>(editor);
    if (listEditor == nullptr)
    {
        // Not an editor create by this class; ignore it
        return QItemDelegate::setEditorData(editor, index);
    }

    auto currentData = index.data(Qt::EditRole).toList();

    for (int i=0; i<currentData.count(); i++)
        listEditor->setChecked(currentData[i].toInt());
    listEditor->scrollToLastChecked();

    return;
}

void ColumnReferenceDelegate::setModelData(QWidget* editor,
                                           QAbstractItemModel* model,
                                           const QModelIndex& index) const
{
    auto listEditor = dynamic_cast<ColumnReferenceSelector*>(editor);
    if (listEditor == nullptr)
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
    QList<int> selectedIDs = listEditor->checkedIDs();

    // Update the model
    QList<QVariant> valueList;
    valueList.reserve(selectedIDs.size());
    for (auto i=selectedIDs.cbegin(); i!=selectedIDs.cend(); i++)
        valueList.push_back(*i);
    table->setData(index, valueList);
    return;
}

void ColumnReferenceDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
    auto listEditor = dynamic_cast<ColumnReferenceSelector*>(editor);
    if (listEditor == nullptr)
        return QItemDelegate::updateEditorGeometry(editor, option, index);

    listEditor->setOptimizedSize();
    listEditor->resize(option.rect.width(), listEditor->height());

    int x = option.rect.x(), y = option.rect.y();
    QWidget* parent = dynamic_cast<QWidget*>(listEditor->parent());
    if (parent)
    {
        if (parent->height() < y + editor->height())
            y = parent->height() - editor->height();
    }
    listEditor->move(x, y);
}

void ColumnReferenceDelegate::onEditorAddingItemRequested(
                                ColumnReferenceSelector* editor, QString text)
{
    int tableID = editor->property(OPENOTE_DELEGATE_EDITOR_PROP_TABLE).toInt();
    int column = editor->property(OPENOTE_DELEGATE_EDITOR_PROP_COL).toInt();
    if (tableID < 1 || column < 0)
        return;

    TableModel* table = book->table(tableID);
    TableModel* referenceTable = book->columnReferenceTable(table->ID, column);

    int rowID;
    int referredColumn = 0;
    bool conversionOK = true, successful = false;
    switch (referenceTable->columnType(referredColumn))
    {
        case TableModel::ColumnType::Integer:
        {
            text = text.trimmed();
            int intValue = text.toInt(&conversionOK);
            if (conversionOK)
            {
                rowID = referenceTable->newRow();
                successful = referenceTable->modify(rowID, referredColumn,
                                                    intValue);
            }
            break;
        }
        case TableModel::ColumnType::Double:
        {
            text = text.trimmed();
            double doubleValue = text.toDouble(&conversionOK);
            if (conversionOK)
            {
                rowID = referenceTable->newRow();
                successful = referenceTable->modify(rowID, referredColumn,
                                                    doubleValue);
            }
            break;
        }
        case TableModel::ColumnType::String:
        {
            std::string stringValue = text.toStdString();
            rowID = referenceTable->newRow();
            successful = referenceTable->modify(rowID, referredColumn,
                                                stringValue);
            break;
        }
        case TableModel::ColumnType::IntegerList:
        {
            int intValue;
            std::vector<int> integers;
            auto values = text.split(',');
            integers.reserve(values.size());
            for (auto i=values.cbegin(); i!=values.cend(); i++)
            {
                intValue = i->toInt(&conversionOK);
                if (!conversionOK)
                    break;
                integers.push_back(intValue);
            }
            if (conversionOK)
            {
                rowID = referenceTable->newRow();
                successful = referenceTable->modify(rowID, referredColumn,
                                                    integers);
            }
            break;
        }
        default:;
    }
    if (!conversionOK)
        QMessageBox::warning(editor, tr("Value format mismatch"),
                             tr("The input text cannot be convert to "
                             "value type of the referred column.\n"
                             "Please try to add rows manually."));
    if (successful)
        editor->addItem(rowID, text);
}
