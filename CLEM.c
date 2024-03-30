#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "main.h"

char partition [20]="test.bin";

// Fonction de formatage de la partition
int myFormat(char* partitionName) {
    int i = 0;
    file f;
    int open_partition = open(partitionName, O_RDWR | O_CREAT, 0666);
    if (open_partition == -1) {
        perror("echec du formatage");
        return -1; // Échec du formatage
    }
    strcpy(partition,partitionName);

    // Ajout de fichier vide dans la partition
    for (int i = 0; i < PARTITION_SIZE; i += BLOCK_SIZE) {
        f.dispo=1;
        f.taille=0;
        f.debut=i;
        f.position=sizeof(file)+i;
        write(open_partition, &f, sizeof(file));
    }
    close(open_partition);
    return 0; // Formatage réussi
}

// Fonction d'ouverture de fichier
file* myOpen(char* fileName) {
    file* new_file=(file*)malloc(sizeof(file));
    file f;
    int i=0;
    int open_partition = open(partition, O_RDWR);
    if (open_partition == -1) {
        perror("echec de l'ouverture");
        return NULL; // Échec de l'ouverture
    }

    read(open_partition, &f, sizeof(file));
    //Parcours de la partition
    while(i<PARTITION_SIZE && f.dispo !=1 && strcmp(f.nom, fileName) != 0){
        read(open_partition, &f, sizeof(file));
        i+=BLOCK_SIZE;
    }    
    if(f.dispo == 1){            
        strcpy(f.nom ,fileName);
        f.dispo = 0;
        lseek(open_partition, f.debut, SEEK_SET);
        write(open_partition, &f, sizeof(file));
    }    
    //Si le fichier n'a pas été trouvé
    else if(i == PARTITION_SIZE){
        perror("partition pleine \n");
        return NULL; // La partition est pleine 
    }    

    *new_file=f;
    close(open_partition);

    return new_file;
}

// Fonction d'écriture dans un fichier
int myWrite(file* f, void* buffer, int nBytes) {
    if (f == NULL || buffer == NULL || nBytes <= 0) {
        perror("parametres invalides");
        return -1; // Paramètres invalides
    }
    int open_partition = open(partition, O_RDWR); // Ouvrez la partition en lecture/écriture
    if (open_partition == -1) {
        perror("echec de l'ouverture");
        return -1; // Échec de l'ouverture
    }

    // Positionnez le curseur au début du fichier dans la partition
    lseek(open_partition, f->position , SEEK_SET);
    int bytes_written = write(open_partition, buffer, nBytes);
    if (bytes_written < 0) {
        perror("erreur d'écriture");
        return -1; // Erreur d'écriture
    }

    f->taille += bytes_written;
    f->position += bytes_written;
    close(open_partition);
    printf("nb bytes ecrit: %d\n",bytes_written);
    return bytes_written;
}

// Fonction de lecture depuis un fichier
int myRead(file* f, void* buffer, int nBytes) {
    if (f == NULL || buffer == NULL || nBytes <= 0) {
        perror("parametres invalides");
        return -1; // Paramètres invalides
    }
    // printf("%d\n",f->debut);
    int open_partition = open(partition, O_RDONLY);
    lseek(open_partition, (f->debut+sizeof(file)), SEEK_SET);
    int bytes_read = read(open_partition, buffer, nBytes);
    if (bytes_read < 0) {
        perror("erreur de lecture");
        return -1; // Erreur de lecture
    }

    close(open_partition);
    return bytes_read;
}

// Fonction de déplacement du curseur
void mySeek(file* f, int offset, int base) {
    int newPosition;
    if (f == NULL || f->dispo == 1) {
        return; // Paramètre invalide
    }

    switch(base){
        case SEEK_SET:
            f->position = f->debut + offset;
            break;
        case SEEK_CUR:
            f->position += offset ;
            break;
        case SEEK_END:
            f->position = f->taille + f->debut;
            break;
        default:
            return; 
    }

}


int main() {
    char buffer1[100]; // Utilisé pour la lecture/écriture des fichiers
    int choice;
    file* opened_file = NULL; // Pour suivre le fichier ouvert, s'il y en a un
    char partitionName[] = "partition.bin";

    while (1) {
        printf("\nMenu:\n");
        printf("1. Initialiser / Réinitialiser la partition\n");
        printf("2. Ouvrir un fichier\n");
        printf("3. Ecrire dans le fichier\n");
        printf("4. Lire un fichier\n");
        printf("5. Donner la taille d'un fichier (en octets)\n");
        printf("6. Quitter\n");
        printf("Choix : ");
        scanf("%d", &choice);
        getchar(); // Pour consommer le caractère de nouvelle ligne

        switch (choice) {
            case 1:
                if (myFormat(partitionName) == 0) {
                    printf("Partition initialisée avec succès.\n");
                } else {
                    printf("Échec de l'initialisation de la partition.\n");
                }
                break;

            case 2:
                
                printf("Entrez le nom du fichier à ouvrir : ");
                scanf("%s", buffer1);
                opened_file = myOpen(buffer1);
                if (opened_file != NULL) {
                    printf("Fichier ouvert avec succès. Emplacement dans la partition : %d\n", opened_file->debut);
                } else {
                    printf("Échec de l'ouverture du fichier.\n");
                }
                break;

            case 3:
                if (opened_file == NULL) {
                    printf("Aucun fichier ouvert.\n");
                    break;
                }
                printf("Entrez le texte à écrire dans le fichier : ");
                scanf("%s", buffer1);
                myWrite(opened_file, buffer1, strlen(buffer1));
                printf("Écriture terminée.\n");
                break;

            case 4:
                if (opened_file == NULL) {
                    printf("Aucun fichier ouvert.\n");
                    break;
                }
                myRead(opened_file, buffer1, sizeof(buffer1));
                printf("Contenu du fichier : %s\n", buffer1);
                break;

            case 5:
                if (opened_file == NULL) {
                    printf("Aucun fichier ouvert.\n");
                    break;
                }
                printf("Taille du fichier : %d octets\n", opened_file->taille);
                break;

            case 6:
                // Quitter le programme
                if (opened_file != NULL) {
                    // Si un fichier est ouvert, le fermer
                    // myClose(opened_file);
                    opened_file = NULL;
                }
                printf("Au revoir !\n");
                exit(0);
                break;

            default:
                printf("Choix invalide. Veuillez entrer un nombre entre 1 et 6.\n");
                break;
        }
    }

    return 0;
}