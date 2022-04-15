#ifndef ONBOOK_H
#define ONBOOK_H

#include "ontable.h"


class ONBookPrivate;

class ONBook
{
public:
    ONBook();
    virtual ~ONBook();

    virtual int count() const;
    virtual bool exists(int tableID) const;
    virtual std::vector<int> tableIDs() const;

    virtual void clear();

    virtual ONTable* table(int tableID) const;
    virtual ONTable* newTable();
    virtual ONTable* addTable(const std::string& tableName);
    virtual bool removeTable(int tableID);

    virtual std::string tableName(int tableID) const;
    virtual bool setTableName(int tableID, const std::string& newName);

    virtual int columnReference(int sourceTableID, int sourceColumnID) const;
    virtual bool setColumnReference(int sourceTableID,
                                    int sourceColumnID,
                                    int targetTableID);
    virtual void removeColumnReference(int sourceTableID, int sourceColumnID);

    virtual std::string bindingDirectory() const;
    virtual bool setBindingDirectory(const std::string& path);
    virtual void clearBindingDirectory();

    virtual bool load();
    virtual bool save();

protected:
    ONBookPrivate* d_ptr;
    ONBook(ONBookPrivate* data);
};

#endif // ONBOOK_H
