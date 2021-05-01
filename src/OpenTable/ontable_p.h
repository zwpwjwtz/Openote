#ifndef ONTABLE_P_H
#define ONTABLE_P_H

#include <list>
#include <vector>
#include <string>
#include "ontablecolumn.h"


class ONTable;

class ONTablePrivate
{
public:
    ONTablePrivate();

    int ID;
    std::string name;

    std::list<int> IDList;
    std::vector<int> columnIDList;
    std::vector<std::string> columnNameList;

    std::vector<ONTableColumn> columnList;

    size_t getColumnIndexByID(int columnID);
};

#endif // ONTABLE_P_H
