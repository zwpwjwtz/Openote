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
        String = 4
    };

    ONTable();
    ~ONTable();

    int ID();
    void setID(int ID);

    std::string name();
    void setName(const std::string& name);

    int countRow();
    int countColumn();

    bool existsRow(int ID);
    bool existsColumn(int ID);

    std::list<int> IDs();
    std::vector<std::string> columnNames();

    void clear();

    int newRow();
    int newColumn(const std::string& name, ColumnType columnType);

    bool modify(int ID, int columnID, const int& value);
    bool modify(int ID, int columnID, const double& value);
    bool modify(int ID, int columnID, const std::string& value);

    void removeRow(int ID);
    void removeColumn(int columnID);

protected:
    ONTablePrivate* d_ptr;
    ONTable(ONTablePrivate* data);
};

#endif // ONTABLE_H
