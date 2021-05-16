#ifndef ONTABLE_H
#define ONTABLE_H

#include <list>
#include <vector>
#include <string>


struct ONTableDefaultValue
{
    int intValue;
    double doubleValue;
    std::string stringValue;
};

class ONTablePrivate;

class ONTable
{
public:
    enum ColumnType
    {
        None = 0,
        Integer = 1,
        Double = 2,
        String = 4,
        IntegerList = 9
    };

    ONTable();
    ONTable(const ONTable& src);
    virtual ~ONTable();

    int ID;

    virtual int countRow() const;
    virtual int countColumn() const;

    virtual bool existsRow(int ID) const;
    virtual bool existsColumn(int ID) const;

    virtual std::list<int> IDs() const;
    virtual std::vector<int> columnIDs() const;

    virtual std::string columnName(int columnID) const;
    virtual void setColumnName(int columnID, const std::string &newName);

    virtual void clear();
    virtual void clearRow(int ID);
    virtual void clearColumn(int columnID);

    virtual int newRow();
    virtual int newColumn(const std::string& name, ColumnType columnType);

    virtual ColumnType columnType(int columnID) const;
    virtual bool setColumnType(int columnID, ColumnType newType);

    virtual ONTableDefaultValue defaultValues() const;
    virtual void setDefaultValues(const ONTableDefaultValue& values);

    virtual int readInt(int ID, int columnID) const;
    virtual double readDouble(int ID, int columnID) const;
    virtual std::string readString(int ID, int columnID) const;
    virtual std::vector<int> readIntList(int ID, int columnID) const;

    virtual bool modify(int ID, int columnID, const int& value);
    virtual bool modify(int ID, int columnID, const double& value);
    virtual bool modify(int ID, int columnID, const std::string& value);
    virtual bool modify(int ID, int columnID,
                        const std::vector<int>& valueList);

    virtual std::list<int> insert(int columnID,
                                  const std::list<int>& valueList);
    virtual std::list<int> insert(int columnID,
                                  const std::list<double>& valueList);
    virtual std::list<int> insert(int columnID,
                                  const std::list<std::string>& valueList);
    virtual std::list<int> insert(int columnID,
                                  const std::list<std::vector<int>>& valueList);

    virtual void removeRow(int ID);
    virtual void removeColumn(int columnID);

    virtual std::string bindingDirectory() const;
    virtual bool setBindingDirectory(const std::string& path);
    virtual void clearBindingDirectory();

    virtual std::string fileSuffix() const;
    virtual void setFileSuffix(const std::string& suffix);

    virtual bool load();
    virtual bool save();

protected:
    ONTablePrivate* d_ptr;
    ONTable(ONTablePrivate* data);
};

#endif // ONTABLE_H
