#ifndef ONBOOK_P_H
#define ONBOOK_P_H

#include <vector>
#include <map>
#include <string>


class ONTable;
class ONBook;

class ONBookPrivate
{
public:
    std::vector<ONTable*> tableList;
    std::vector<int> tableIDList;
    std::vector<std::string> tableNameList;

    std::map<std::pair<int, int>, int> columnReference;

    std::string bindingDirectory;

    virtual ~ONBookPrivate();

    virtual int getTableIndexByID(int tableID) const;
    virtual std::string getIndexFilename() const;
    virtual std::string getTableDirectory(int tableID) const;

    virtual bool loadTable(int tableID, const std::string& tableName);
    virtual bool saveTable(int tableIndex);
    virtual bool addColumnReferences(int targetTableID,
                                     const std::string& referenceMapString);
};

#endif // ONBOOK_P_H
