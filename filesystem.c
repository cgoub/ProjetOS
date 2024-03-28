#include "filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PARTITION_SIZE 1024 * 1024 * 10
#define BLOCK_SIZE 512
#define META_BLOCKS 1

FileSystem fs;
const char* partitionName = "filesystem.img";

int myFormat(const char* partitionName) {
    FILE* partitionFile = fopen(partitionName, "wb");
    if (!partitionFile) {
        perror("Failed to open partition file");
        return -1;
    }

    // init meta
    FileSystemMeta meta;
    meta.block_size = BLOCK_SIZE;
    meta.total_blocks = PARTITION_SIZE / BLOCK_SIZE;
    // calculater the bloc disponible
    meta.free_blocks = meta.total_blocks - META_BLOCKS - (BITMAP_SIZE / BLOCK_SIZE + ((BITMAP_SIZE % BLOCK_SIZE) ? 1 : 0));

    // write
    if (fwrite(&meta, sizeof(FileSystemMeta), 1, partitionFile) != 1) {
        perror("Failed to write file system meta to partition");
        fclose(partitionFile);
        return -1;
    }

    // init controle
    uint8_t* bitmap = (uint8_t*)malloc(BITMAP_SIZE);
    memset(bitmap, 0xFF, BITMAP_SIZE); // 将所有位初始化为1，表示空闲

    // write after meta
    if (fwrite(bitmap, 1, BITMAP_SIZE, partitionFile) != BITMAP_SIZE) {
        perror("Failed to write block bitmap to partition");
        fclose(partitionFile);
        free(bitmap);
        return -1;
    }
    free(bitmap); // free the buffer

    // finish the rest
    fseek(partitionFile, PARTITION_SIZE - 1, SEEK_SET);
    fputc('\0', partitionFile); // 在文件的最后位置写入一个字节

    fclose(partitionFile);
    return 0;
}

int loadBitmap(FileSystemMeta* meta, const char* partitionName) {
    
}

void initFileSystem(FileSystem* fs, const char* partitionName) {
    
}

FileBlock* allocateFileBlock(FileMeta* meta, uint32_t blockNumber) {
    
}

DirectoryEntry* createEntry(FileSystem* fs, DirectoryEntry* parent, const char* name, int isDirectory) {
   
}

file* myOpen(const char* fileName) {
    // chercher de la racine
    DirectoryEntry* parent = fs.root; //  fs est global 
    DirectoryEntry* entry = findEntry(parent, fileName);

    if (entry == NULL) {
        // s'il existe pas ,creer
        entry = createEntry(&fs, parent, fileName, 0); // 0 est le fichier
        if (entry == NULL) {
            fprintf(stderr, "Failed to create file: %s\n", fileName);
            return NULL;
        }
    }

    // entry pas NULL，pointer  DirectoryEntry
    // creer file 
    file* f = (file*)malloc(sizeof(file));
    if (!f) {
        fprintf(stderr, "Memory allocation failed for file structure.\n");
        return NULL;
    }

    f->meta = entry->fileMeta;
    f->currentBlock = entry->fileMeta->firstBlock; // 
    f->currentPosition = 0;

    return f;
}

int myWrite(file* f, void* buffer, int nBytes) {
 
}

int myRead(file* f, void* buffer, int nBytes) {
   
}
