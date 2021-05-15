#include "columnreferencedelegate.h"
#include "models/bookmodel.h"
#include "models/tablemodel.h"
#include "columnreferenceselector.h"
#include <QListView>
#include <QMessageBox>

#define OPENOTE_DELEGATE_EDITOR_PROP_ID    "column-reference"
#define OPENOTE_DELEGATE_EDITOR_PROP_TABLE "table-id"
#define OPENOTE_DELEGATE_EDITOR_PROP_COL   "column-id"


ColumnReferenceDelegate::ColumnReferenceDelegate()
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

    int columnID = table->columnID(index.column());
    const TableModel* referenceTable = book->columnReferenceTable(table->ID,
                                                                  columnID);
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

    ColumnReferenceSelector* listEditor = new ColumnReferenceSelector(parent);
    listEditor->setProperty(OPENOTE_DELEGATE_EDITOR_PROP_ID, true);
    listEditor->setProperty(OPENOTE_DELEGATE_EDITOR_PROP_TABLE, table->ID);
    listEditor->setProperty(OPENOTE_DELEGATE_EDITOR_PROP_COL, columnID);
    connect(listEditor, &ColumnReferenceSelector::addingItemRequested,
            this, &ColumnReferenceDelegate::onEditorAddingItemRequested);
    for (auto i=rowIDs.cbegin(); i!=rowIDs.cend(); i++)
    {
        switch (columnType)
        {
            case TableModel::ColumnType::Integer:
                listEditor->addItem(*i,
                            referenceTable->readInt(*i, referredColumnID));
                break;
            case TableModel::ColumnType::Double:
                listEditor->addItem(*i,
                            referenceTable->readDouble(*i, referredColumnID));
                break;
            case TableModel::ColumnType::String:
                listEditor->addItem(*i,
                    QString::fromStdString(
                            referenceTable->readString(*i, referredColumnID)));
                break;
            case TableModel::ColumnType::IntegerList:
            {
                QString displayedString;
                auto values = referenceTable->readIntList(*i, referredColumnID);
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
    if (listEditor == nullptr ||
        !listEditor->property(OPENOTE_DELEGATE_EDITOR_PROP_ID).isValid())
    {
        // Not an editor create by this class; ignore it
        return QItemDelegate::setEditorData(editor, index);
    }

    auto currentData = index.data(Qt::EditRole).toList();

    for (int i=0; i<currentData.count(); i++)
        listEditor->setChecked(currentData[i].toInt());
    return;
}

void ColumnReferenceDelegate::setModelData(QWidget* editor,
                                           QAbstractItemModel* model,
                                           const QModelIndex& index) const
{
    auto listEditor = dynamic_cast<ColumnReferenceSelector*>(editor);
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
    QList<int> selectedIDs = listEditor->checkedIDs();

    // Update the model
    QList<QVariant> valueList;
    valueList.reserve(selectedIDs.size());
    for (auto i=selectedIDs.cbegin(); i!=selectedIDs.cend(); i++)
        valueList.push_back(*i);
    table->setData(index, valueList);
    return;
}

void ColumnReferenceDelegate::onEditorAddingItemRequested(
                                ColumnReferenceSelector* editor, QString text)
{
    int tableID = editor->property(OPENOTE_DELEGATE_EDITOR_PROP_TABLE).toInt();
    int columnID = editor->property(OPENOTE_DELEGATE_EDITOR_PROP_COL).toInt();
    if (tableID < 1 || columnID < 1)
        return;

    TableModel* table = book->table(tableID);
    TableModel* referenceTable = book->columnReferenceTable(table->ID,
                                                            columnID);

    int rowID;
    int referredColumnID = referenceTable->columnID(0);
    bool conversionOK = true, successful = false;
    switch (referenceTable->columnType(referredColumnID))
    {
        case TableModel::ColumnType::Integer:
        {
            text = text.trimmed();
            int intValue = text.toInt(&conversionOK);
            if (conversionOK)
            {
                rowID = referenceTable->newRow();
                successful = referenceTable->modify(rowID, referredColumnID,
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
                successful = referenceTable->modify(rowID, referredColumnID,
                                                    doubleValue);
            }
            break;
        }
        case TableModel::ColumnType::String:
        {
            std::string stringValue = text.toStdString();
            rowID = referenceTable->newRow();
            successful = referenceTable->modify(rowID, referredColumnID,
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
                successful = referenceTable->modify(rowID, referredColumnID,
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
