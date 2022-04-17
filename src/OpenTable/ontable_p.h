#ifndef ONTABLE_P_H
#define ONTABLE_P_H

#include <list>
#include <vector>
#include <string>
#include "ontablecolumn.h"


class ONTablePrivate
{
public:
    ONTablePrivate();
    ONTablePrivate(const ONTablePrivate& src);
    virtual ~ONTablePrivate();

    std::string bindingDirectory;
    std::string fileSuffix;

    std::list<int> IDList;
    std::vector<int> columnIDList;
    std::vector<int> columnTypeIDList;
    std::vector<std::string> columnNameList;

    std::vector<ONTableColumn*> columnList;

    int defaultIntValue;
    double defaultDoubleValue;
    std::string defaultStringValue;

    int getColumnIndexByID(int columnID) const;
    std::string getIndexFilename() const;
    std::string getColumnFilename(int columnID) const;

    virtual bool loadColumn(int columnID, int columnType,
                            const std::string& columnName);
    virtual bool saveColumn(int columnIndex) const;

    virtual bool loadIDList();
    virtual bool saveIDList() const;
};

#endif // ONTABLE_P_H
