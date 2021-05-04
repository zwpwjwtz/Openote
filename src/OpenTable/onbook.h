#ifndef ONBOOK_H
#define ONBOOK_H

#include <string>
#include "ontable.h"


class ONBookPrivate;

class ONBook
{
public:
    ONBook();
    ~ONBook();

    int count() const;
    bool exists(int tableID) const;

    void clear();

    ONTable* table(int tableID) const;
    int addTable(const std::string& tableName);
    bool setTableName(int tableID, const std::string& newName);
    void removeTable(int tableID);

    int columnReference(int sourceTableID, int sourceColumnID) const;
    bool setColumnReference(int sourceTableID,
                            int sourceColumnID,
                            int targetTableID);
    void removeColumnReference(int sourceTableID, int sourceColumnID);

    std::string bindingDirectory() const;
    bool setBindingDirectory(const std::string& path);

    bool load();
    bool save();

protected:
    ONBookPrivate* d_ptr;
    ONBook(ONBookPrivate* data);
};

#endif // ONBOOK_H
