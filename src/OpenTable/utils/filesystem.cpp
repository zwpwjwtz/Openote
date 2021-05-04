#include <errno.h>
#include <sys/stat.h>
#include "filesystem.h"


bool utils_isDirectory(const char* path)
{
    struct stat fileInfo;
    return stat(path, &fileInfo) == 0 && S_ISDIR(fileInfo.st_mode);
}

bool utils_isFile(const char* path)
{
    struct stat fileInfo;
    return stat(path, &fileInfo) == 0 && S_ISREG(fileInfo.st_mode);
}

long utils_fileLength(const char* filename)
{
    struct stat fileInfo;
    if (stat(filename, &fileInfo) == 0)
        return fileInfo.st_size;
    else
        return 0;
}

bool utils_newDirectory(const char* path)
{
    struct stat fileInfo;
    if (stat(path, &fileInfo) == 0)
    {
        // The path is valid; see if it is a directory
        if (S_ISDIR(fileInfo.st_mode))
            return true;
        else
        {
            // Path occupied by non-directory file
            // We can do nothing about it
            return false;
        }
    }
    else
    {
        if (errno == ENOENT)
        {
            // The path does not exist; create it
            mkdir(path, S_IRWXU);
            return true;
        }
        else
        {
            // Other types of error (access denied, etc.)
            return false;
        }
    }
}
