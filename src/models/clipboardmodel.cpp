#ifdef QT_GUI_LIB
#include <QGuiApplication>
#include <QClipboard>
#endif
#include "clipboardmodel.h"


ClipboardModel::ClipboardModel()
{
    currentType = Unknown;

    data = nullptr;
    dataList = nullptr;
    dataLength = 0;
    dataListLength = 0;
}

ClipboardModel::~ClipboardModel()
{
    if (data != nullptr)
        operator delete(data);

    if (dataList != nullptr)
        operator delete[](dataList);
}

bool ClipboardModel::canPaste() const
{
#ifdef QT_GUI_LIB
    if (!QGuiApplication::clipboard()->text().isEmpty())
        return true;
#endif
    return data != nullptr || dataList != nullptr;
}

int ClipboardModel::contentCount() const
{
#ifdef QT_GUI_LIB
    if (!QGuiApplication::clipboard()->text().isEmpty())
        return 1;
#endif
    switch (currentType)
    {
        case Integer:
        case Double:
            return data == nullptr ? 0 : 1;
        case String:
            return dataListLength / sizeof(char);
        case IntegerList:
            return dataListLength / sizeof(int);
        default:
            return dataListLength;
    }
}

ClipboardModel::ContentType ClipboardModel::contentType() const
{
#ifdef QT_GUI_LIB
    bool conversionOK;
    QString text = QGuiApplication::clipboard()->text();
    if (text.isEmpty())
        return Unknown;

    text.toInt(&conversionOK);
    if (conversionOK)
        return ContentType::Integer;
    text.toDouble(&conversionOK);
    if (conversionOK)
        return ContentType::Double;
    return ContentType::String;
#else
    return currentType;
#endif
}

void ClipboardModel::clear()
{
#ifdef QT_GUI_LIB
    if (!QGuiApplication::clipboard()->text().isEmpty())
        QGuiApplication::clipboard()->clear();
#endif
    if (data)
    {
        operator delete(data);
        dataLength = 0;
    }
    if (dataList)
    {
        operator delete[](dataList);
        dataListLength = 0;
    }
}

void ClipboardModel::copy(int intValue)
{
#ifdef QT_GUI_LIB
    QGuiApplication::clipboard()->setText(QString::number(intValue));
#else
    if (data)
        operator delete(data);

    currentType = Integer;
    dataLength = sizeof(int);
    data = operator new(dataLength);
    memcpy(data, &intValue, dataLength);
#endif
}

void ClipboardModel::copy(double doubleValue)
{
#ifdef QT_GUI_LIB
    QGuiApplication::clipboard()->setText(QString::number(doubleValue));
#else
    if (data)
        operator delete(data);

    currentType = Double;
    dataLength = sizeof(double);
    data = operator new(dataLength);
    memcpy(data, &doubleValue, dataLength);
#endif
}

void ClipboardModel::copy(const std::string& stringValue)
{
#ifdef QT_GUI_LIB
    QGuiApplication::clipboard()->setText(
                                    QString::fromStdString(stringValue));
#else
    if (dataList)
        operator delete[](dataList);

    currentType = String;
    dataListLength = sizeof(char) * stringValue.size();
    dataList = operator new[](dataListLength);
    memcpy(dataList, stringValue.c_str(), dataListLength);
#endif
}

void ClipboardModel::copy(const std::vector<int>& intList)
{
#ifdef QT_GUI_LIB
    QGuiApplication::clipboard()->clear();
#endif
    if (dataList)
        operator delete[](dataList);

    currentType = IntegerList;
    dataListLength = sizeof(char) * intList.size();
    dataList = operator new[](dataListLength);
    memcpy(dataList, intList.data(), dataListLength);
}

int ClipboardModel::pasteAsInt()
{
#ifdef QT_GUI_LIB
    return QGuiApplication::clipboard()->text().toInt();
#else
    int value = 0;
    if (data != nullptr && dataLength >= sizeof(int))
        memcpy(&value, data, sizeof(int));
    else if (dataList != nullptr && dataListLength >= sizeof(int))
        memcpy(&value, dataList, sizeof(int));
    return value;
#endif
}

double ClipboardModel::pasteAsDouble()
{
#ifdef QT_GUI_LIB
    return QGuiApplication::clipboard()->text().toDouble();
#else
    double value = 0;
    if (data != nullptr && dataLength >= sizeof(double))
        memcpy(&value, data, sizeof(double));
    else if (dataList != nullptr && dataListLength >= sizeof(double))
        memcpy(&value, dataList, sizeof(double));
    return value;
#endif
}

std::string ClipboardModel::pasteAsString()
{
#ifdef QT_GUI_LIB
    return QGuiApplication::clipboard()->text().toStdString();
#else
    std::string value;
    if (dataList != nullptr)
    {
        value.assign(std::string(reinterpret_cast<char*>(dataList),
                                 dataListLength / sizeof(char)));
    }
    return value;
#endif
}

std::vector<int> ClipboardModel::pasteAsIntList()
{
    std::vector<int> valueList;
    if (dataList != nullptr)
    {
        int listLength = int(dataListLength / sizeof(int));
        valueList.reserve(listLength);
        memcpy(valueList.data(), dataList, listLength * sizeof(int));
    }
    return valueList;
}
