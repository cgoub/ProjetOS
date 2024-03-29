#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_FILES 100 // Nombre maximum de fichiers dans la partition
#define BLOCK_SIZE 1024 // Taille de bloc pour notre partition
#define PARTITION_SIZE 32 * 1024 * 1024 // Taille de la partition en octets (32 Mo)
#define PARTITION_NAME "myPartition.bin"

// Calcul du nombre total de blocs necessaires
int totalBlocks = PARTITION_SIZE / BLOCK_SIZE;

typedef struct {
    char * fileName;
    int fileSize;
    int startBlock;
    int isOpen;
    int isUsed;
    int type; // 1 pour simple fichier et 0 pour repertoire
    // Cursor state
    int cursor;
} file;

int myFormat(const char* partitionName) {
    int partition = open(partitionName, O_RDWR | O_CREAT, 0666);
    if (partition == -1) {
        return -1;
    }

    if(partition == 0){
        // Initialisation de la table des fichiers avec des entrÃ©es vides
        file emptyFile = {0};
        for (int i = 0; i < MAX_FILES; i++) {
            write(partition, &emptyFile, sizeof(file));
        }
        // Remplissage du reste de la partition avec des blocs vides
        emptyFile.isUsed = 0;
        for (int i = MAX_FILES * sizeof(file); i < PARTITION_SIZE; i += BLOCK_SIZE) {
            write(partition, &emptyFile, sizeof(file));
        }
    }
    

    close(partition);
    return 0;
}

file *myOpen(const char* partitionName, const char* fileName) {
    int partition = open(partitionName, O_RDWR);
    if (partition == -1) {
        return NULL;
    }

    file f;
    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        lseek(partition, i * sizeof(file), SEEK_SET);
        read(partition, &f, sizeof(file));
        if (f.isUsed && strcmp(f.fileName, fileName) == 0) {
            found = 1;
            f.isOpen = 1;
            lseek(partition, -sizeof(file), SEEK_CUR);
            write(partition, &f, sizeof(file));
            break;
        }
    }

    if (!found) {
        lseek(partition, 0, SEEK_SET);
        for (int i = 0; i < MAX_FILES; i++) {
            read(partition, &f, sizeof(file));
            if (!f.isUsed) {
                strncpy(f.fileName, fileName, sizeof(f.fileName) - 1);
                f.fileSize = 0;
                f.startBlock = MAX_FILES * sizeof(file) + i * BLOCK_SIZE;
                f.isOpen = 1;
                f.isUsed = 1;
                lseek(partition, -sizeof(file), SEEK_CUR);
                lseek(partition, -sizeof(file), SEEK_CUR);
                write(partition, &f, sizeof(file));
                break;
            }
        }
    }

    close(partition);

    file* openedFile = (file*)malloc(sizeof(file));
    if (openedFile != NULL) {
        *openedFile = f; 
    }
    return openedFile;
}

void mySeek(file* f, int offset, int base) {
    if (f == NULL || !f->isOpen) {
        return; // Ã‰chec du dÃ©placement
    }

    int newPosition;

    switch (base) {
        case SEEK_SET:
            newPosition = offset;
            break;
        case SEEK_CUR:
            newPosition = f->fileSize + offset;
            break;
        case SEEK_END:
            newPosition = f->fileSize + offset;
            break;
        default:
            return; // Ã‰chec du dÃ©placement pour base invalide
    }

    // S'assurer que la position ne devient pas nÃ©gative
    if (newPosition < 0) {
        newPosition = 0; 
    }

    // VÃ©rifier si la nouvelle position ne dÃ©passe pas la taille maximale du fichier
    f->fileSize = newPosition;
}

int myRead(file* f, void* buffer, int nBytes) {
    if (f == NULL || buffer == NULL || !f->isOpen) {
        return -1; // Ã‰chec de la lecture
    }

    int partition = open(PARTITION_NAME, O_RDONLY);
    if (partition == -1) {
        perror("Error opening partition file");
        return -1; // Ã‰chec de l'ouverture de la partition
    }

    mySeek(f, 0, SEEK_SET); // Se positionne au dÃ©but du fichier

    // Se positionne Ã  la position actuelle de lecture dans le fichier
    mySeek(f, f->fileSize, SEEK_SET);

    // Lecture des donnÃ©es depuis le fichier vers le tampon
    int bytesRead = read(partition, buffer, nBytes);

    // Met Ã  jour la position actuelle de lecture dans le fichier
    mySeek(f, f->fileSize + bytesRead, SEEK_SET);

    close(partition);

    return bytesRead;
}

int myWrite(file* f, void* buffer, int nBytes) {
    if (f == NULL || buffer == NULL || !f->isOpen) {
        return -1; // Ã‰chec de l'Ã©criture
    }

    int partition = open(PARTITION_NAME, O_WRONLY);
    if (partition == -1) {
        perror("Error opening partition file");
        return -1; // Ã‰chec de l'ouverture de la partition
    }

    mySeek(f, 0, SEEK_SET); // Se positionne au dÃ©but du fichier

    // Se positionne Ã  la position actuelle de lecture dans le fichier
    mySeek(f, f->fileSize, SEEK_SET);

    // Ã‰criture des donnÃ©es depuis le tampon vers le fichier
    int bytesWritten = write(partition, buffer, nBytes);

    // Met Ã  jour la position actuelle de lecture dans le fichier
    mySeek(f, f->fileSize + bytesWritten, SEEK_SET);

    close(partition);

    return bytesWritten;
}

void myLs(const char* partitionName) {
    int partition = open(partitionName, O_RDONLY);
    if (partition == -1) {
        perror("Error opening partition file");
        return;
    }

    file f;
    lseek(partition, 0, SEEK_SET);
    printf("Liste des fichiers dans la partition :\n");
    for (int i = 0; i < MAX_FILES; i++) {
        read(partition, &f, sizeof(file));
        if (f.isUsed) {
            printf("Nom du fichier: %s\n", f.fileName);
        }
    }

    close(partition);
}

int main() {
    // Formatage de la partition
    if (myFormat(PARTITION_NAME) != 0) {
        printf("Erreur lors du formatage de la partition.\n");
        return 1;
    }

    char fileName[] = "test.txt";

    file* f = myOpen(PARTITION_NAME, fileName);
    if (f == NULL) {
        printf("Erreur lors de l'ouverture du fichier.\n");
        return 1;
    }

    printf("Fichier '%s' ouvert avec succes. Taille: %d, Bloc de debut: %d\n", f->fileName, f->fileSize, f->startBlock);

    char readBuffer[50];
    char buffer[] = "Hello, world!";

    int bytesWritten = myWrite(f, buffer, strlen(buffer));
    int bytesRead = myRead(f, readBuffer, sizeof(readBuffer));
    
    printf("Contenu du fichier : %s\n", readBuffer);

    myLs(PARTITION_NAME);

    free(f); // LibÃƒÂ¨re la mÃ©moire allouÃ©e pour le fichier
    return 0;
}