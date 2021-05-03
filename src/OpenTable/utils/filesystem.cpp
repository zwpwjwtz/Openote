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
