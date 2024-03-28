#ifndef FILESYSTEM_API_H
#define FILESYSTEM_API_H

#include "filesystem_structs.h"

// La bibliothèque à construire
int myFormat(const char* partitionName);
file* myOpen(const char* fileName);
int myWrite(file* f, void* buffer, int nBytes);
int myRead(file* f, void* buffer, int nBytes);
void mySeek(file* f, int offset, int base);

#endif // FILESYSTEM
