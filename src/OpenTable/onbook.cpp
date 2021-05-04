#include <cstring>
#include <sstream>
#include "onbook.h"
#include "onbook_p.h"
#include "utils/filesystem.h"

#define ONTABLE_BOOK_INDEX_FILE_NAME      "onbook-index"
#define ONTABLE_BOOK_INDEX_FILE_HEADER    "ONTBOOKINDEX"
#define ONTABLE_BOOK_INDEX_FILE_VERSION   "1.0\n"

#define ONTABLE_BOOK_IO_BUFFER_MAXLEN     256


ONBook::ONBook()
{
    d_ptr = new ONBookPrivate;
}

ONBook::ONBook(ONBookPrivate* data)
{
    if (data == nullptr)
        ONBook();
    else
        d_ptr = data;
}

ONBook::~ONBook()
{
    delete d_ptr;
}

int ONBook::count() const
{
    return int(d_ptr->tableList.size());
}

bool ONBook::exists(int tableID) const
{
    return d_ptr->getTableIndexByID(tableID) >= 0;
}

void ONBook::clear()
{
    d_ptr->tableList.clear();
    d_ptr->tableIDList.clear();
    d_ptr->tableNameList.clear();
}

ONTable* ONBook::table(int tableID) const
{
    int index = d_ptr->getTableIndexByID(tableID);
    if (index >= 0)
        return &d_ptr->tableList[index];
    else
        return nullptr;
}

int ONBook::addTable(const std::string& tableName)
{
    // Assuming increasing ID of tables in the list
    int availableID = d_ptr->tableList.size() > 0 ?
                      d_ptr->tableList.back().ID + 1 :
                      1;
    d_ptr->tableList.push_back(ONTable());
    d_ptr->tableList.back().ID = availableID;
    d_ptr->tableIDList.push_back(availableID);
    d_ptr->tableNameList.push_back(tableName);

    return availableID;
}

bool ONBook::setTableName(int tableID, const std::string& newName)
{
    int index = d_ptr->getTableIndexByID(tableID);
    if (index >= 0)
    {
        d_ptr->tableNameList[index] = newName;
        return true;
    }
    else
        return false;
}

void ONBook::removeTable(int tableID)
{
    int index = d_ptr->getTableIndexByID(tableID);
    if (index < 0)
        return;

    d_ptr->tableList.erase(d_ptr->tableList.begin() + index);
    d_ptr->tableIDList.erase(d_ptr->tableIDList.begin() + index);
    d_ptr->tableNameList.erase(d_ptr->tableNameList.begin() + index);
}

int ONBook::columnReference(int sourceTableID, int sourceColumnID) const
{
    auto pos = d_ptr->columnReference.find(std::make_pair(sourceTableID,
                                                          sourceColumnID));
    if (pos == d_ptr->columnReference.cend())
        return -1;
    else
        return pos->second;
}

bool ONBook::setColumnReference(int sourceTableID,
                                int sourceColumnID,
                                int targetTableID)
{
    int sourceIndex = d_ptr->getTableIndexByID(sourceTableID);
    int targetIndex = d_ptr->getTableIndexByID(targetTableID);
    if (sourceIndex < 0 || targetIndex < 0 ||
        !d_ptr->tableList[sourceIndex].existsColumn(sourceColumnID))
        return false;
    if (d_ptr->tableList[sourceIndex].columnType(sourceColumnID) !=
        ONTable::ColumnType::Integer)
        return false;

    auto key = std::make_pair(sourceTableID, sourceColumnID);
    auto pos = d_ptr->columnReference.find(key);
    if (pos == d_ptr->columnReference.end())
        d_ptr->columnReference.insert(std::make_pair(key, targetTableID));
    else
        pos->second = targetTableID;
    return true;
}

void ONBook::removeColumnReference(int sourceTableID, int sourceColumnID)
{
    auto pos = d_ptr->columnReference.find(std::make_pair(sourceTableID,
                                                          sourceColumnID));
    if (pos != d_ptr->columnReference.cend())
        d_ptr->columnReference.erase(pos);
}

std::string ONBook::bindingDirectory() const
{
    return d_ptr->bindingDirectory;
}

bool ONBook::setBindingDirectory(const std::string& path)
{
    // TODO: backup the binding directory for all tables
    //       before setting the new ones

    if (!utils_isDirectory(path.c_str()))
        return false;
    d_ptr->bindingDirectory = path;

    // Set binding sub-directories for each table
    // Create directories if needed
    std::stringstream subdir;
    for (size_t i=0; i<d_ptr->tableList.size(); i++)
    {
        subdir.str("");
        subdir << path << "/" << d_ptr->tableIDList[i];
        if (!utils_newDirectory(subdir.str().c_str()))
            return false;
        d_ptr->tableList[i].setBindingDirectory(subdir.str());
    }
    return true;
}

bool ONBook::load()
{
    if (d_ptr->bindingDirectory.empty())
        return false;

    // Read column information from the index file
    std::string indexFilename = d_ptr->getIndexFilename();
    if (!utils_isFile(indexFilename.c_str()))
        return false;

    char buffer[ONTABLE_BOOK_IO_BUFFER_MAXLEN];
    FILE* f = fopen(indexFilename.c_str(), "rb");

    // Check for the index file header
    fread(buffer, 1, strlen(ONTABLE_BOOK_INDEX_FILE_HEADER), f);
    if (strncmp(buffer, ONTABLE_BOOK_INDEX_FILE_HEADER,
                strlen(ONTABLE_BOOK_INDEX_FILE_HEADER)) != 0)
    {
        fclose(f);
        return false;
    }
    // Ignore version check
    fseek(f, strlen(ONTABLE_BOOK_INDEX_FILE_VERSION), SEEK_CUR);

    clear();

    char* pos, *posS, *posE;
    int tableID;
    std::string tableName;
    while (!feof(f))
    {
        if (fgets(buffer, ONTABLE_BOOK_IO_BUFFER_MAXLEN - 1, f) != buffer)
            break;

        // Parse the table index, the table name,
        // and the column references from each line
        tableID = static_cast<int>(strtol(buffer, &pos, 10));
        if (buffer == pos)
            continue;

        posS = strstr(pos + 1, "\"");
        if (!posS)
            continue;
        posE = strstr(posS + 1, "\"");
        if (!posE)
            continue;
        tableName = std::string(posS + 1, posE - posS - 1);

        posS = strstr(posE + 1, "[");
        if (!posS)
            continue;
        posE = strstr(posS + 1, "]");
        if (!posE)
            continue;
        d_ptr->addColumnReferences(tableID,
                                   std::string(posS + 1, posE - posS - 1));

        if (!d_ptr->loadTable(tableID, tableName))
        {
            fclose(f);
            return false;
        }
    }
    fclose(f);
    return true;
}

bool ONBook::save()
{
    size_t i;
    for (i=0; i<d_ptr->tableList.size(); i++)
    {
        if (!d_ptr->tableList[i].save())
            return false;
    }

    // Update the index file
    FILE* f = fopen(d_ptr->getIndexFilename().c_str(), "wb");
    if (!f)
        return false;
    fputs(ONTABLE_BOOK_INDEX_FILE_HEADER, f);
    fputs(ONTABLE_BOOK_INDEX_FILE_VERSION, f);

    std::stringstream columnReferences;
    for (i=0; i<d_ptr->tableList.size(); i++)
    {
        // Get ID of columns referenced to each table
        columnReferences.str("");
        for (auto j = d_ptr->columnReference.cbegin();
             j != d_ptr->columnReference.cend(); j++)
        {
            if (j->second == d_ptr->tableIDList[i])
            {
                if (!columnReferences.str().empty())
                    columnReferences << ",";
                columnReferences << j->first.first
                                 << ":"
                                 << j->first.second;
            }
        }
        fprintf(f, "%d,\"%s\",[%s]\n",
                d_ptr->tableIDList[i],
                d_ptr->tableNameList[i].c_str(),
                columnReferences.str().c_str());
    }

    fclose(f);
    return true;
}

int ONBookPrivate::getTableIndexByID(int tableID) const
{
    for (size_t i=0; i<tableIDList.size(); i++)
    {
        if (tableIDList[i] == tableID)
            return int(i);
    }
    return -1;
}

std::string ONBookPrivate::getIndexFilename() const
{
    std::string indexFilename;

    if (bindingDirectory.empty() ||
        !utils_isDirectory(bindingDirectory.c_str()))
        return indexFilename;
    indexFilename.append(bindingDirectory)
                 .append("/")
                 .append(ONTABLE_BOOK_INDEX_FILE_NAME);
    return indexFilename;
}

std::string ONBookPrivate::getTableDirectory(int tableID) const
{
    std::string directory;
    if (bindingDirectory.empty() ||
        !utils_isDirectory(bindingDirectory.c_str()))
        return directory;

    char ID[ONTABLE_BOOK_IO_BUFFER_MAXLEN];
    sprintf(ID, "%d", tableID);
    directory.append(bindingDirectory)
             .append("/")
             .append(ID);
    return directory;
}

bool ONBookPrivate::loadTable(int tableID, const std::string& tableName)
{
    tableList.push_back(ONTable());
    ONTable& table = tableList.back();
    if (!(table.setBindingDirectory(getTableDirectory(tableID)) &&
          table.load()))
        return false;

    table.ID = tableID;
    tableIDList.push_back(tableID);
    tableNameList.push_back(tableName);

    return true;
}

bool ONBookPrivate::saveTable(int tableIndex)
{
    // TODO: save bindingFilename for each columns before saving
    ONTable& table = tableList[tableIndex];
    std::string oldDirectory;
    std::stringstream directory;
    oldDirectory = table.bindingDirectory() == "" ?
                   "" :
                   table.bindingDirectory();
    directory.str("");
    directory << bindingDirectory << "/" << table.ID;
    table.setBindingDirectory(directory.str());
    if (table.save())
        return true;
    else
    {
        table.setBindingDirectory(oldDirectory);
        return false;
    }
}

bool ONBookPrivate::addColumnReferences(int targetTableID,
                                        const std::string& referenceMapString)
{
    size_t pos1, pos2 = 0;
    int tableID, columnID;
    const char* posS;
    char* posE;
    while (true)
    {
        // Parse the table ID
        pos1 = referenceMapString.find(':', pos2);
        if (pos1 == std::string::npos)
            break;
        posS = referenceMapString.c_str() + pos2;
        tableID = int(strtol(posS, &posE, 10));
        if (posE == posS)
            break;

        // Parse the column ID
        pos2 = pos1 + 1;
        pos1 = referenceMapString.find(':', pos2);
        if (pos1 == std::string::npos)
        {
            pos1 = referenceMapString.length() - 1;
            if (pos1 <= pos2)
                break;
        }
        posS = referenceMapString.c_str() + pos2;
        columnID = int(strtol(posS, &posE, 10));
        if (posE == posS)
            break;

        columnReference.insert(std::make_pair(std::make_pair(tableID, columnID),
                                              targetTableID));
    }
    return true;
}
