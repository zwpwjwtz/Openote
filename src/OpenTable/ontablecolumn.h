#ifndef ONTABLECOLUMN_H
#define ONTABLECOLUMN_H

#define ONTABLE_COLUMN_TYPE_BASE    0


class ONTableColumnPrivate;

class ONTableColumn
{
public:
    int ID;
    int typeID;

    ONTableColumn();
    ONTableColumn(const ONTableColumn& src);
    virtual ~ONTableColumn();

    void clear() const;
    int length() const;

    virtual bool exists(int key) const;
    virtual char* value(int key) const;
    virtual void set(int key, char* value);
    virtual void remove(int key);

    virtual const char* bindingFile();
    virtual bool setBindingFile(const char* filename);
    virtual bool load() { return false; }
    virtual bool save() { return false; }

protected:
    ONTableColumnPrivate* d_ptr;
    ONTableColumn(ONTableColumnPrivate* data);
};

#endif // ONTABLECOLUMN_H
