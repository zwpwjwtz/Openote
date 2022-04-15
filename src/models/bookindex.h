#ifndef BOOKINDEX_H
#define BOOKINDEX_H


struct BookIndex
{
    int table = -1;
    int column = -1;
    int row = -1;
    bool operator ==(const BookIndex& src)
    { return table == src.table && column == src.column && row == src.row; }
    bool isValid()
    { return table >= 0 && column >= 0 && row >= 0; }
    void reset()
    { table = column = row = -1; }
};


#endif // BOOKINDEX_H
