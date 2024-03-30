#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "main.h"

char partition [20]="partition.bin";

/**
 * @brief Fonction permettant le formatage/l'ouverture d'une partition
 * 
 * @param partitionName Nom de la partition a créer
 * @return int Renvoie -1 si il y a une erreur, renvoie 0 sinon.
 */
int myFormat(char* partitionName) {
    file f;
    int j=1;
    int open_partition = open(partitionName, O_RDWR | O_CREAT, 0666);
    if (open_partition == -1) {
        return -1; // Échec du formatage
    }
    strcpy(partition,partitionName);

    // Ajout de fichier vide dans la partition
    f.dispo=1;
    f.taille=0;
    for (int i = 0; i < PARTITION_SIZE; i += BLOCK_SIZE) {
        f.debut=(j-1) * sizeof(file) + i;
        f.position= j * sizeof(file) + i;
        write(open_partition, &f, sizeof(file));
        lseek(open_partition,BLOCK_SIZE,SEEK_CUR);
        j++;
    }    
    close(open_partition);
    return 0; // Formatage réussi
}

/**
 * @brief Fonction qui permet d'ouverir un fichier dans la partition choisie.
 * 
 * @param fileName Nom du fichier à ouvrir
 * @return file* Renvoie le fichier ouvert, renvoie NULL sinon
 */
file* myOpen(char* fileName) {
    file* new_file=(file*)malloc(sizeof(file));
    file f;
    int i=0;
    int open_partition = open(partition, O_RDWR);
    if (open_partition == -1) {
        return NULL; // Échec de l'ouverture
    }
    lseek(open_partition,0,SEEK_SET);
    read(open_partition, &f, sizeof(file));
    //Parcours de la partition
    while(i<PARTITION_SIZE && f.dispo !=1 && strcmp(f.nom, fileName) != 0){
        i+=BLOCK_SIZE;
        lseek(open_partition, BLOCK_SIZE, SEEK_CUR);
        read(open_partition, &f, sizeof(file));
    }    
    if(f.dispo == 1){            
        strcpy(f.nom ,fileName);
        f.dispo = 0;
        lseek(open_partition, -sizeof(file), SEEK_CUR);
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

/**
 * @brief Fonction permettant d'afficher les informations d'un fichier en particulier.
 * 
 * @param fileName Nom du fichier à analyser.
 * @return int Renvoie -1 en cas d'erreur, renvoie 0 sinon.
 */
int showInfo(char * fileName){
    int i;
    int open_partition = open(partition, O_RDWR);
    file f;
    if (open_partition == -1) {
        return -1; // Échec de l'ouverture
    }    
    lseek(open_partition, 0, SEEK_SET);
    read(open_partition, &f, sizeof(file));
    while(i<PARTITION_SIZE && strcmp(f.nom, fileName) != 0){
        lseek(open_partition, BLOCK_SIZE, SEEK_CUR);
        read(open_partition, &f, sizeof(file));
        i+=BLOCK_SIZE;
    }    
    if(i<PARTITION_SIZE){  
        printf("Nom du fichier : %s\n", f.nom);
        printf("Taille du fichier : %d octets\n", f.taille);
        printf("Emplacement dans la partition : %d\n", f.debut);
        return 0;          
    }    
    return -1; // Le fichier n'existe pas
}

/**
 * @brief Affiche tous les fichiers ouverts dans la partition.
 * 
 * @return int Renvoie 0 si tout c'est bien déroulé, renvoie -1 sinon.
 */
int showFiles(){
    file f;
    int i=0;
    int j=1;
    int open_partition = open(partition, O_RDWR);
    if (open_partition == -1) {
        return -1; // Échec de l'ouverture
    }

    lseek(open_partition, 0, SEEK_SET);
    read(open_partition, &f, sizeof(file));
    //Parcours de la partition
    while(i<PARTITION_SIZE &&  f.dispo ==0){
        printf("\nFichier %d: %s\n",j,f.nom);
        printf("Taille : %d\n",f.taille);
        lseek(open_partition, BLOCK_SIZE, SEEK_CUR);
        read(open_partition, &f, sizeof(file));
        j++;
        i+=BLOCK_SIZE;
    }    
    if(i==0){        
        printf("Aucun fichier trouvé.\n");
    }    
    else{
        printf("\nNombre total de fichier: %d\n",j-1);
    }
    return 0;
}

/**
 * @brief Fonction permettant d'écrire dans un fichier appartenant à la partition.
 * 
 * @param f Le fichier qu'on veut éditer.
 * @param buffer Le texte à insérer dans le fichier.
 * @param nBytes Le nombre de caractères à insérer.
 * @return int Renvoie 0 si tout c'est bien déroulé, renvoie -1 sinon.
 */
int myWrite(file* f, void* buffer, int nBytes) {
    if (f == NULL || buffer == NULL || nBytes <= 0) {
        return -1; // Paramètres invalides
    }
    int open_partition = open(partition, O_RDWR); // Ouvrez la partition en lecture/écriture
    if (open_partition == -1) {
        return -1; // Échec de l'ouverture
    }

    // Positionnez le curseur au début du fichier dans la partition
    lseek(open_partition, f->position , SEEK_SET);
    int bytes_written = write(open_partition, buffer, nBytes);
    if (bytes_written < 0) {
        return -1; // Erreur d'écriture
    }

    f->taille += bytes_written;
    f->position += bytes_written;
    lseek(open_partition,f->debut,SEEK_SET);
    write(open_partition,f,sizeof(file));
    close(open_partition);
    printf("nb bytes ecrit: %d\n",bytes_written);
    return bytes_written;
}

/**
 * @brief Fonction permettant de lire un fichier entré en paramètre.
 * 
 * @param f Le fichier qu'on veut lire.
 * @param buffer Le texte à lu est stocké dans le buffer.
 * @param nBytes Le nombre de caractères à lire.
 * @return int Renvoie 0 si tout c'est bien déroulé, renvoie -1 sinon.
 */
int myRead(file* f, void* buffer, int nBytes) {
    if (f == NULL || buffer == NULL || nBytes <= 0) {
        return -1; // Paramètres invalides
    }
    int open_partition = open(partition, O_RDONLY);
    lseek(open_partition, (f->debut+sizeof(file)), SEEK_SET);
    int bytes_read = read(open_partition, buffer, nBytes);
    if (bytes_read < 0) {
        return -1; // Erreur de lecture
    }

    close(open_partition);
    return bytes_read;
}

/**
 * @brief Fonction permettant de déplacer l'emplacement de la tête de lecture d'un fichier.
 * 
 * @param f Le fichier dans lequel on souhaite se déplacer
 * @param offset La distance à laquelle on souhaite se déplacer
 * @param base Le parèmetre en fonction du quel on veut se déplacer (SEEK_SET, SEEK_CUR, SEEK_END).
 */
void mySeek(file* f, int offset, int base) {
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
    int init=0;

    while(init!=1){
        printf("Choisissez la partition à utiliseravec \".bin\" \n");
        scanf("%s", buffer1);
        if (myFormat(buffer1) == 0) {
            printf("Partition initialisée avec succès.\n");
            init=1;
        } else {
            printf("Échec de l'initialisation de la partition.\n");
    }
    }
    while (1) {
        printf("\nMenu:\n");
        printf("1. Initialiser / Réinitialiser la partition\n");
        printf("2. Ouvrir un fichier\n");
        printf("3. Ecrire dans le fichier\n");
        printf("4. Lire un fichier\n");
        printf("5. Afficher les informations d'un fichier \n");
        printf("6. Afficher la liste des fichiers\n");
        printf("7. Quitter\n");
        printf("Choix : ");
        scanf("%d", &choice);
        getchar(); // Pour consommer le caractère de nouvelle ligne

        switch (choice) {
            case 1:
                
                printf("Entrez le nom de la partition a créer/formater avec \".bin\" \n");
                scanf("%s", buffer1);
                if (myFormat(buffer1) == 0) {
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
                scanf("%[^\n]s", buffer1);
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
                printf("Entrez le nom du fichier à analyser : ");
                scanf("%s", buffer1);
                
                if(showInfo(buffer1)==0){
                    printf("Affichage terminé.");
                    break;
                }
                printf("Le fichier est introuvable.");
                break;
            
            case 6:
                showFiles();
                break;

            case 7:
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
