#ifndef CLIPBOARDMODEL_H
#define CLIPBOARDMODEL_H

#include <string>
#include <vector>


class ClipboardModel
{
public:
    enum ContentType
    {
        Unknown = 0,
        Integer = 1,
        Double = 2,
        String = 4,
        IntegerList = 9
    };

    ClipboardModel();
    ~ClipboardModel();

    bool canPaste() const;

    int contentCount() const;
    ContentType contentType() const;

    void clear();

    void copy(int intValue);
    void copy(double doubleValue);
    void copy(const std::string& stringValue);
    void copy(const std::vector<int>& intList);

    int pasteAsInt();
    double pasteAsDouble();
    std::string pasteAsString();
    std::vector<int> pasteAsIntList();

private:
    ContentType currentType;

    void* data;
    int dataLength;
    void* dataList;
    int dataListLength;
};

#endif // CLIPBOARDMODEL_H
