#include <QKeyEvent>
#include "columnreferenceselector.h"
#include "ui_columnreferenceselector.h"

#define OPENOTE_REFSELECTOR_ITEM_ROLE_SEARCH Qt::UserRole + 2


ColumnReferenceSelector::ColumnReferenceSelector(QWidget* parent) :
    QFrame(parent),
    ui(new Ui::ColumnReferenceSelector)
{
    listModel.setColumnCount(2);

    ui->setupUi(this);
    setLayout(ui->verticalLayout);
    ui->listView->setModel(&listModel);
    ui->listView->horizontalHeader()
                    ->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->listView->horizontalHeader()->setStretchLastSection(true);
}

ColumnReferenceSelector::~ColumnReferenceSelector()
{
    delete ui;
}

void ColumnReferenceSelector::focusInEvent(QFocusEvent* event)
{
    Q_UNUSED(event)
    ui->textSearch->setFocus();
}

void ColumnReferenceSelector::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case Qt::Key_Up:
            if (ui->listView->hasFocus())
                ui->textSearch->setFocus();
            break;
        case Qt::Key_Down:
            if (ui->textSearch->hasFocus())
                ui->listView->setFocus();
            break;
        default:
            QFrame::keyPressEvent(event);
    }
}

int ColumnReferenceSelector::count() const
{
    return listModel.rowCount();
}

void ColumnReferenceSelector::clear()
{
    listModel.setRowCount(0);
    IDList.clear();
}

void ColumnReferenceSelector::addItem(int ID, const QString& text)
{
    QStandardItem* checkbox = new QStandardItem(" ");
    checkbox->setCheckable(true);
    checkbox->setEditable(false);
    checkbox->setSelectable(false);
    QStandardItem* item = new QStandardItem(text);
    item->setData(text.toLower(), OPENOTE_REFSELECTOR_ITEM_ROLE_SEARCH);
    item->setCheckable(false);
    item->setEditable(false);
    listModel.appendRow(QList<QStandardItem*>() << checkbox << item);
    IDList.push_back(ID);
}

void ColumnReferenceSelector::addItem(int ID, int intValue)
{
    QStandardItem* checkbox = new QStandardItem(" ");
    checkbox->setCheckable(true);
    checkbox->setEditable(false);
    checkbox->setSelectable(false);
    QStandardItem* item = new QStandardItem(QString::number(intValue));
    item->setData(QString::number(intValue),
                  OPENOTE_REFSELECTOR_ITEM_ROLE_SEARCH);
    item->setCheckable(true);
    item->setEditable(false);
    listModel.appendRow(QList<QStandardItem*>() << checkbox << item);
    IDList.push_back(ID);
}

void ColumnReferenceSelector::addItem(int ID, double doubleValue)
{
    QStandardItem* checkbox = new QStandardItem(" ");
    checkbox->setCheckable(true);
    checkbox->setEditable(false);
    checkbox->setSelectable(false);
    QStandardItem* item = new QStandardItem(QString::number(doubleValue));
    item->setData(QString::number(doubleValue),
                  OPENOTE_REFSELECTOR_ITEM_ROLE_SEARCH);
    item->setCheckable(true);
    item->setEditable(false);
    listModel.appendRow(QList<QStandardItem*>() << checkbox << item);
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

void ColumnReferenceSelector::scrollToTop()
{
    ui->listView->scrollToTop();
}

void ColumnReferenceSelector::scrollToBottom()
{
    ui->listView->scrollToBottom();
}

void ColumnReferenceSelector::scrollToItem(int ID)
{
    int index = IDList.indexOf(ID);
    if (index >= 0)
        ui->listView->scrollTo(listModel.index(index, 0));
}

void ColumnReferenceSelector::scrollToFirstChecked()
{
    for (int i=0; i<listModel.rowCount(); i++)
    {
        if (listModel.item(i, 0)->checkState() == Qt::Checked)
        {
            ui->listView->scrollTo(listModel.index(i, 0));
            break;
        }
    }
}

void ColumnReferenceSelector::scrollToLastChecked()
{
    for (int i=listModel.rowCount()-1; i>=0; i--)
    {
        if (listModel.item(i, 0)->checkState() == Qt::Checked)
        {
            ui->listView->scrollTo(listModel.index(i, 0));
            break;
        }
    }
}

void ColumnReferenceSelector::on_textSearch_textChanged()
{
    const QString searchText(ui->textSearch->text().toLower());
    ui->buttonAdd->setEnabled(!searchText.isEmpty());

    int count = listModel.rowCount();
    for (int i=0; i<count; i++)
    {
        ui->listView->setRowHidden(i,
                                   !listModel.item(i, 1)
                                   ->data(OPENOTE_REFSELECTOR_ITEM_ROLE_SEARCH)
                                   .toString().contains(searchText));
    }
}

void ColumnReferenceSelector::on_buttonAdd_clicked()
{
    emit addingItemRequested(this, ui->textSearch->text());
}
