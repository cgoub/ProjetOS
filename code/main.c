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
    f.dispo=1;
    f.taille=0;
    for (int i = 0; i < PARTITION_SIZE; i += BLOCK_SIZE) {
        f.debut=sizeof(file)+i;
        f.position=sizeof(file)+i;
        write(open_partition, &f, sizeof(file));
    }
    close(open_partition);
    return 0; // Formatage réussi
}

// Fonction d'ouverture de fichier
file* myOpen(char* fileName) {
    file* new_file=(file*)malloc(sizeof(file));
    int i=0;
    int open_partition = open(partition, O_RDWR);
    if (open_partition == -1) {
        perror("echec de l'ouverture");
        return NULL; // Échec de l'ouverture
    }

    //read(open_partition, new_file, sizeof(file));
    //Parcours de la partition
    while(i<PARTITION_SIZE && new_file->dispo !=1 && strcmp(new_file->nom, fileName) != 0){
        lseek(open_partition, i , SEEK_SET);
        read(open_partition, new_file, sizeof(file));
        i+=BLOCK_SIZE;
    }
    printf("%d",i);

    if(new_file->dispo == 1){            
        strcpy(new_file->nom ,fileName);
        new_file->dispo = 0;
        write(open_partition, new_file, sizeof(file));
    }    
    //Si le fichier n'a pas été trouvé
    else if(i == PARTITION_SIZE){
        perror("partition pleine \n");
        return NULL; // La partition est pleine 
    }    

    printf("dispo %d\n",new_file->dispo);
    printf("debut %d\n",new_file->debut);
    printf("nom %s\n",new_file->nom);
    
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
    printf("%d\n",f->debut);
    int open_partition = open(partition, O_RDONLY);
    lseek(open_partition, f->debut, SEEK_SET);
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

// // Fonction de fermeture de fichier
// void myClose(file* f) {
//     if (f == NULL) {
//         return; // Paramètre invalide
//     }

//     free(f->nom);
//     free(f);
// }

// Fonction de test basique
int main() {
    char buffer1[10];
    myFormat("test.bin");
    file* test=myOpen("test.txt");
    //myRead(test,buffer1,10);
    printf("myread test1: %s\n",buffer1);
    
    file* test2=myOpen("test2.txt");
    //myRead(test2,buffer1,10);
    printf("nom test2: %s\n",test2->nom);
    printf("myRead test2: %s\n",buffer1);

    char write[15]="bjr c'est moi";
    myWrite(test,write,15);
    myRead(test,buffer1,10);
    // printf("nom test1 :%s\n",test->nom);
    printf("myRead 3: %s\n",buffer1);


    char buffer2[10];
    char write2[15]="hello world";
    myWrite(test2,write2,15);
    myRead(test2,buffer2,10);
    printf("myRead 4: %s\n",buffer2);

    myRead(test,buffer1,10);
    printf("myRead 5: %s\n",buffer1);

    // // Test de création de fichier
    // if (myFormat("partition.txt") != 0) {
    //     printf("Erreur lors du formatage de la partition.\n");
    //     return 1;
    // }

    // // Test d'ouverture de fichier
    // file* test_file = myOpen("test.txt");
    // if (test_file == NULL) {
    //     printf("Erreur lors de l'ouverture du fichier.\n");
    //     return 1;
    // }

    // // Test d'écriture dans le fichier
    // char buffer[20] = "Hello, world!";
    // if (myWrite(test_file, buffer, strlen(buffer)) != strlen(buffer)) {
    //     printf("Erreur lors de l'écriture dans le fichier.\n");
    //     return 1;
    // }

    // // Test de lecture depuis le fichier
    // char read_buffer[20];
    // mySeek(test_file, 0, SEEK_SET);
    // if (myRead(test_file, read_buffer, strlen(buffer)) != strlen(buffer)) {
    //     printf("Erreur lors de la lecture depuis le fichier.\n");
    //     return 1;
    // }
    // read_buffer[strlen(buffer)] = '\0'; // Ajout du caractère de fin de chaîne
    // printf("Contenu du fichier : %s\n", read_buffer);

    // // Fermeture du fichier
    // myClose(test_file);

    return 0;
}
