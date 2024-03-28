#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PARTITION_SIZE 1000000 // Taille de la partition en octets

// Structure de fichier
typedef struct {
    char* filename;
    int cursor; // Pointeur de lecture/écriture
} file;

// Déclaration de la partition en tant que tableau de caractères
char partition[PARTITION_SIZE];

// Fonction de formatage de la partition
int myFormat(char* partitionName) {
    // Initialisation de la partition avec des zéros
    memset(partition, 0, PARTITION_SIZE);
    printf("Partition '%s' formatée avec succès.\n", partitionName);
    return 0;
}

// Fonction d'ouverture ou création de fichier
file* myOpen(char* fileName) {
    file* newFile = (file*)malloc(sizeof(file));
    if (newFile == NULL) {
        printf("Erreur: Impossible d'allouer de la mémoire pour le fichier.\n");
        return NULL;
    }

    newFile->filename = fileName;
    newFile->cursor = 0;
    printf("Fichier '%s' ouvert ou créé avec succès.\n", fileName);
    return newFile;
}

// Fonction d'écriture dans un fichier
int myWrite(file* f, void* buffer, int nBytes) {
    // Vérification de la limite de la partition
    if (f->cursor + nBytes > PARTITION_SIZE) {
        printf("Erreur: Dépassement de la taille de la partition.\n");
        return -1;
    }

    // Copie des données dans la partition
    memcpy(partition + f->cursor, buffer, nBytes);
    f->cursor += nBytes;
    return nBytes;
}

// Fonction de lecture depuis un fichier
int myRead(file* f, void* buffer, int nBytes) {
    // Vérification de la limite de la partition
    if (f->cursor + nBytes > PARTITION_SIZE) {
        printf("Erreur: Tentative de lecture au-delà de la taille de la partition.\n");
        return -1;
    }

    // Copie des données depuis la partition
    memcpy(buffer, partition + f->cursor, nBytes);
    f->cursor += nBytes;
    return nBytes;
}

// Fonction de déplacement du pointeur de lecture/écriture
void mySeek(file* f, int offset, int base) {
    if (base == SEEK_SET)
        f->cursor = offset;
    else if (base == SEEK_CUR)
        f->cursor += offset;
    else if (base == SEEK_END)
        f->cursor = PARTITION_SIZE - offset;
}

// Programme de test minimal
int main() {
    // Formatage de la partition
    myFormat("DisqueDur");

    // Création et ouverture d'un fichier
    file* myFile = myOpen("monfichier.txt");

    // Écriture dans le fichier
    char data[] = "Bonjour, monde !";
    myWrite(myFile, data, strlen(data));

    // Lecture depuis le fichier
    char readData[100];
    mySeek(myFile, 0, SEEK_SET); // Se positionner au début du fichier
    myRead(myFile, readData, strlen(data));
    readData[strlen(data)] = '\0'; // Ajout de la terminaison de chaîne

    printf("Contenu du fichier : %s\n", readData);

    // Fermeture du fichier
    free(myFile);

    return 0;
}
