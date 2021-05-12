#include "columnreferenceselector.h"
#include "ui_columnreferenceselector.h"


ColumnReferenceSelector::ColumnReferenceSelector(QWidget* parent) :
    QFrame(parent),
    ui(new Ui::ColumnReferenceSelector)
{
    ui->setupUi(this);
    setLayout(ui->verticalLayout);
    ui->listView->setModel(&listModel);
    ui->listView->setModelColumn(0);
}

void ColumnReferenceSelector::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)
    setOptimizedSize();
}

int ColumnReferenceSelector::count() const
{
    return listModel.rowCount();
}

void ColumnReferenceSelector::clear()
{
    listModel.clear();
    IDList.clear();
}

void ColumnReferenceSelector::addItem(int ID, const QString& text)
{
    QStandardItem* item = new QStandardItem(text);
    item->setCheckable(true);
    item->setEditable(false);
    listModel.appendRow(item);
    IDList.push_back(ID);
}

void ColumnReferenceSelector::addItem(int ID, int intValue)
{
    QStandardItem* item = new QStandardItem(QString::number(intValue));
    item->setCheckable(true);
    item->setEditable(false);
    listModel.appendRow(item);
    IDList.push_back(ID);
}

void ColumnReferenceSelector::addItem(int ID, double doubleValue)
{
    QStandardItem* item = new QStandardItem(QString::number(doubleValue));
    item->setCheckable(true);
    item->setEditable(false);
    listModel.appendRow(item);
    IDList.push_back(ID);
}

QList<int> ColumnReferenceSelector::checkedIDs() const
{
    QList<int> IDs;
    for (int i=0; i<listModel.rowCount(); i++)
    {
        if (listModel.item(i, 0)->checkState() == Qt::Checked)
            IDs.push_back(IDList[i]);
    }
    return IDs;
}

void ColumnReferenceSelector::setChecked(int ID, bool checked)
{
    int index = IDList.indexOf(ID);
    if (index < 0)
        return;
    listModel.item(index, 0)->setCheckState(checked ?
                                            Qt::Checked :
                                            Qt::Unchecked);
}

void ColumnReferenceSelector::setOptimizedSize(int visibleCount)
{
    if (visibleCount <= 0 || visibleCount > 100)
        visibleCount = 5;

    // Calculate the ideal height for the editor
    QSize itemSize =
            ui->listView->itemDelegate()->sizeHint(QStyleOptionViewItem(),
                                                   QModelIndex());
    int height = ui->textSearch->height()
                 + itemSize.height() * visibleCount
                 + ui->verticalLayout->spacing();

    resize(width(), height);
}

void ColumnReferenceSelector::on_textSearch_textChanged()
{
    const QString searchText(ui->textSearch->text());
    ui->buttonAdd->setEnabled(!searchText.isEmpty());

    int count = listModel.rowCount();
    for (int i=0; i<count; i++)
    {
        ui->listView->setRowHidden(
                    i, !listModel.item(i, 0)->text().contains(searchText));
    }
}

void ColumnReferenceSelector::on_buttonAdd_clicked()
{
    emit addingItemRequested(this, ui->textSearch->text());
}