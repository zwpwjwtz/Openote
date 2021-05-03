#ifndef FILESYSTEM_H
#define FILESYSTEM_H


// Return true if the given path is a directory
bool utils_isDirectory(const char* path);

// Return true if the given path is a regular file
bool utils_isFile(const char* path);

// Return the length of a file
long utils_fileLength(const char* filename);

#endif // FILESYSTEM_H
