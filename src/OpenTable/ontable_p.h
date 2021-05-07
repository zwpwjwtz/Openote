#ifndef ONTABLE_P_H
#define ONTABLE_P_H

#include <list>
#include <vector>
#include <string>
#include "ontable.h"
#include "ontablecolumn.h"


class ONTablePrivate
{
public:
    ONTablePrivate();
    ONTablePrivate(const ONTablePrivate& src);

    std::string bindingDirectory;
    std::string fileSuffix;

    std::list<int> IDList;
    std::vector<int> columnIDList;
    std::vector<int> columnTypeIDList;
    std::vector<std::string> columnNameList;

    std::vector<ONTableColumn*> columnList;

    virtual int getColumnIndexByID(int columnID) const;
    virtual std::string getIndexFilename() const;
    virtual std::string getColumnFilename(int columnID) const;

    virtual bool loadColumn(int columnID, int columnType,
                            const std::string& columnName);
    virtual bool saveColumn(int columnIndex);

    static ONTable::ColumnType getColumnType(int columnID,
                                             const std::string& indexFilename);
};

#endif // ONTABLE_P_H
