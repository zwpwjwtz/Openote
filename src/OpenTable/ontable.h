#ifndef ONTABLE_H
#define ONTABLE_H

#include <list>
#include <vector>
#include <string>


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
    ~ONTable();

    int ID;

    int countRow() const;
    int countColumn() const;

    bool existsRow(int ID) const;
    bool existsColumn(int ID) const;

    std::list<int> IDs() const;
    std::vector<int> columnIDs() const;

    std::string columnName(int columnID) const;
    void setColumnName(int columnID, const std::string &newName);

    void clear();

    int newRow();
    int newColumn(const std::string& name, ColumnType columnType);

    ColumnType columnType(int columnID) const;
    int readInt(int ID, int columnID) const;
    double readDouble(int ID, int columnID) const;
    std::string readString(int ID, int columnID) const;
    std::vector<int> readIntList(int ID, int columnID) const;

    bool modify(int ID, int columnID, const int& value);
    bool modify(int ID, int columnID, const double& value);
    bool modify(int ID, int columnID, const std::string& value);
    bool modify(int ID, int columnID, const std::vector<int>& valueList);

    void removeRow(int ID);
    void removeColumn(int columnID);

    std::string bindingDirectory() const;
    bool setBindingDirectory(const std::string& path);
    std::string fileSuffix() const;
    void setFileSuffix(const std::string& suffix);

    bool load();
    bool save();

protected:
    ONTablePrivate* d_ptr;
    ONTable(ONTablePrivate* data);
};

#endif // ONTABLE_H
