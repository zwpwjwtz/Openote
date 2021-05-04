#ifndef ONBOOK_P_H
#define ONBOOK_P_H

#include <vector>
#include <map>
#include <string>
#include "ontable.h"


class ONBook;

class ONBookPrivate
{
public:
    std::vector<ONTable> tableList;
    std::vector<int> tableIDList;
    std::vector<std::string> tableNameList;

    std::map<std::pair<int, int>, int> columnReference;

    std::string bindingDirectory;

    int getTableIndexByID(int tableID) const;
    std::string getIndexFilename() const;
    std::string getTableDirectory(int tableID) const;

    bool loadTable(int tableID, const std::string& tableName);
    bool saveTable(int tableIndex);
    bool addColumnReferences(int targetTableID,
                             const std::string& referenceMapString);
};

#endif // ONBOOK_P_H
