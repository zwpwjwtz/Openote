#ifndef ONTABLECOLUMN_P_H
#define ONTABLECOLUMN_P_H

#include <map>


class ONTableColumn;

class ONTableColumnPrivate
{
public:
    ONTableColumnPrivate();
    ONTableColumnPrivate(const ONTableColumnPrivate& src);

    char* bindingFile;
    std::map<int, char*> data;
};

#endif // ONTABLECOLUMN_P_H
