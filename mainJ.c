#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILES 100 // Nombre maximum de fichiers dans la partition
#define BLOCK_SIZE 4096 // Taille de bloc pour notre partition, simplification

// Mise à jour de la structure file pour inclure isUsed
typedef struct {
    char fileName[100]; // Nom du fichier
    int fileSize;       // Taille du fichier
    int startBlock;     // Bloc de début dans la partition
    int isOpen;         // Indicateur si le fichier est ouvert
    int isUsed;         // Indicateur si l'entrée est utilisée (nouveau)
} file;

int myFormat(char* partitionName) {
    FILE* partition = fopen(partitionName, "wb+"); // Ouvre le fichier en mode écriture/lecture binaire
    if (partition == NULL) {
        return -1;
    }

    // Initialisation de la table des fichiers avec des entrées vides
    file emptyFile = {0};
    for (int i = 0; i < MAX_FILES; i++) {
        fwrite(&emptyFile, sizeof(file), 1, partition);
    }

    fclose(partition);
    return 0;
}

file *myOpen(char* partitionName, char* fileName) {
    FILE* partition = fopen(partitionName, "rb+");
    if (partition == NULL) {
        return NULL;
    }

    file f;
    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        fread(&f, sizeof(file), 1, partition);
        if (f.isUsed && strcmp(f.fileName, fileName) == 0) {
            found = 1;
            f.isOpen = 1;
            fseek(partition, -sizeof(file), SEEK_CUR);
            fwrite(&f, sizeof(file), 1, partition);
            break;
        }
    }

    if (!found) {
        fseek(partition, 0, SEEK_SET);
        for (int i = 0; i < MAX_FILES; i++) {
            fread(&f, sizeof(file), 1, partition);
            if (!f.isUsed) {
                strncpy(f.fileName, fileName, sizeof(f.fileName) - 1);
                f.fileSize = 0;
                f.startBlock = MAX_FILES * sizeof(file) + i * BLOCK_SIZE;
                f.isOpen = 1;
                f.isUsed = 1;
                fseek(partition, -sizeof(file), SEEK_CUR);
                fwrite(&f, sizeof(file), 1, partition);
                break;
            }
        }
    }

    fclose(partition);

    file* openedFile = (file*)malloc(sizeof(file));
    if (openedFile != NULL) {
        *openedFile = f; // Copie les informations du fichier
    }
    return openedFile;
}

int main() {
    char* partitionName = "myPartition.bin";
    char* fileName = "testFile.txt";

    // Formatage de la partition
    if (myFormat(partitionName) != 0) {
        printf("Erreur lors du formatage de la partition.\n");
        return 1;
    }

    // Ouverture (création) d'un fichier
    file* f = myOpen(partitionName, fileName);
    if (f == NULL) {
        printf("Erreur lors de l'ouverture du fichier.\n");
        return 1;
    }

    printf("Fichier '%s' ouvert avec succès. Taille: %d, Bloc de début: %d\n", f->fileName, f->fileSize, f->startBlock);

    free(f); // Libère la mémoire allouée pour le fichier
    return 0;
}