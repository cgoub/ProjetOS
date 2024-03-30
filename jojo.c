#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_FILES 100
#define BLOCK_SIZE 2048 // Taille de bloc pour notre partition
#define PARTITION_SIZE 32 * 1024 * 1024 // Taille de la partition en octets (32 Mo)
#define PARTITION_NAME "partition.bin"
#define nameSize 20


// Calcul du nombre total de blocs nécessaires
int totalBlocks = PARTITION_SIZE / BLOCK_SIZE;

typedef struct {
    char fileName[nameSize];
    int fileSize;
    int startBlock;
    int isOpen;
    int isUsed;
    int cursor;
} file;

int sizeOfEmptyFile(){
    file f;
    return sizeof(f.fileName) + 5 * sizeof(int);
}

int myFormat(const char* partitionName) {
    int partition = open(partitionName, O_RDWR | O_CREAT, 0666);
    if (partition == -1) {
        return -1;
    }

    // Initialisation de la table des fichiers avec des entrées vides
    file emptyFile = {0};
    for (int i = 0; i < totalBlocks; i++) {
        write(partition, &emptyFile, sizeof(file));
    }

    // Remplissage du reste de la partition avec des blocs vides
    emptyFile.isUsed = 0;
    for (int i = MAX_FILES * sizeof(file); i < PARTITION_SIZE; i += BLOCK_SIZE) {
        write(partition, &emptyFile, sizeof(file));
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
        lseek(partition, i * BLOCK_SIZE, SEEK_SET);
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
                f.fileSize = sizeOfEmptyFile();
                //f.startBlock = MAX_FILES * sizeof(file) + i * BLOCK_SIZE;
                f.startBlock = i * BLOCK_SIZE;                
                f.isOpen = 1;
                f.isUsed = 1;
                f.cursor = f.startBlock + sizeOfEmptyFile();
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

void visualisation(const char* partitionName){
    int partition = open(partitionName, O_RDWR);
    int counter = 0;
    if (partition == -1) {
        perror("Erreur lors de l'ouverture de la partition\n");
    }

    file f;
    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        lseek(partition, i * BLOCK_SIZE, SEEK_SET);
        read(partition, &f, sizeof(file));
        if (f.isUsed) {
            counter ++;
        }
    }
    printf("Espace utilisé : %d / %d \n", counter * BLOCK_SIZE, PARTITION_SIZE);

}

void mySeek(file* f, int offset, int base) {
    if (f == NULL || !f->isOpen) {
        return; 
    }

    int newPosition;

    switch (base) {
        case SEEK_SET:
            newPosition = offset + f->startBlock + (sizeOfEmptyFile());
            break;
        case SEEK_CUR:
            newPosition = f->cursor + offset ;
            break;
        case SEEK_END:
            newPosition = f->fileSize + f->startBlock + offset;
            break;
        default:
            return; // Échec du déplacement pour base invalide
    }

    // S'assurer que la position ne devient pas négative
    if (newPosition < 0) {
        newPosition = 0; 
    }

    // Mettre à jour la taille du fichier
    f->cursor = newPosition;
}

int myRead(file* f, void* buffer, int nBytes) {
    if (f == NULL || buffer == NULL || !f->isOpen) {
        return -1; 
    }

    int partition = open(PARTITION_NAME, O_RDONLY);
    if (partition == -1) {
        perror("Error opening partition file");
        return -1; 
    }

   
    mySeek(f, 0, SEEK_SET);
    lseek(partition, 0, SEEK_SET);
    //lseek(partition, f->startBlock + sizeOfEmptyFile(), SEEK_SET);
    lseek(partition, f->cursor, SEEK_SET);

    // Lecture des données depuis le fichier vers le tampon
    int bytesRead = read(partition, buffer, nBytes);

    // Met à jour la position actuelle de lecture dans le fichier
    mySeek(f, 0, SEEK_SET);

    close(partition);

    return bytesRead;
}


int myWrite(file* f, void* buffer, int nBytes) {
    if (f == NULL || buffer == NULL || !f->isOpen) {
        return -1; // Échec de l'écriture
    }

    int partition = open(PARTITION_NAME, O_RDWR); // Ouvrez la partition en lecture/écriture
    if (partition == -1) {
        perror("Error opening partition file");
        return -1; // Échec de l'ouverture de la partition
    }

    // Positionnez le curseur au début du fichier dans la partition
    lseek(partition, f->cursor, SEEK_SET);

    // Écrivez les nouvelles données dans le fichier
    int bytesWritten = write(partition, buffer, nBytes);
    if (bytesWritten == -1) {
        perror("Error writing to file");
        close(partition);
        return -1; // Échec de l'écriture
    }

    // Mettez à jour la taille du fichier dans les métadonnées
    f->fileSize += bytesWritten;
    

    // Positionnez le curseur au début du fichier dans la partition pour écrire les métadonnées
    lseek(partition, f->startBlock, SEEK_SET);

    // Écrivez les métadonnées mises à jour dans la partition
    write(partition, f, sizeof(file));
    f->cursor += bytesWritten;

    close(partition); 

    return bytesWritten;
}

int size(file* f){
    if (f == NULL) {
        printf("Erreur lors de l'ouverture du fichier.\n");
        return 1;
    }
    return f->fileSize - sizeOfEmptyFile();
}




int main() {
    //Formatage de la partition
    
    // if (myFormat(PARTITION_NAME) != 0) {
    //     printf("Erreur lors du formatage de la partition.\n");
    //     return 1;
    // }

    char fileName[] = "fyile.txt";

    // Ouverture (création) d'un fichier
    file* f = myOpen(PARTITION_NAME, fileName);
    if (f == NULL) {
        printf("Erreur lors de l'ouverture du fichier.\n");
        return 1;
    }

    printf("Fichier '%s' ouvert avec succès. Taille: %d, Bloc de début: %d\n", f->fileName, f->fileSize, f->startBlock);

    char readBuffer[50];
    
    myWrite(f, "No stresso", 10);
    mySeek(f, 2, SEEK_SET);
   
   

    myRead(f, readBuffer, sizeof(readBuffer));
    printf("Contenu du fichier : %s\n", readBuffer);

    printf("Taille: %d,",size(f));

    visualisation(PARTITION_NAME);

    free(f); // Libère la mémoire allouée pour le fichier
    return 0;
}