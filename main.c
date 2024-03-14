#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

// Structure de fichier
typedef struct {
    char* filename;
    int cursor_position;
    int file_descriptor;
} file;

// Fonction de formatage de la partition
int myFormat(char* partitionName) {
    int partition = open(partitionName, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (partition == -1) {
        return -1; // Échec du formatage
    }
    close(partition);
    return 0; // Formatage réussi
}

// Fonction d'ouverture de fichier
file* myOpen(char* fileName) {
    file* new_file = malloc(sizeof(file));
    if (new_file == NULL) {
        return NULL; // Échec d'allocation mémoire
    }

    new_file->filename = strdup(fileName);
    new_file->cursor_position = 0;
    new_file->file_descriptor = open(fileName, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (new_file->file_descriptor == -1) {
        free(new_file->filename);
        free(new_file);
        return NULL; // Échec d'ouverture de fichier
    }

    return new_file;
}

// Fonction d'écriture dans un fichier
int myWrite(file* f, void* buffer, int nBytes) {
    if (f == NULL || buffer == NULL || nBytes <= 0) {
        return -1; // Paramètres invalides
    }

    int bytes_written = write(f->file_descriptor, buffer, nBytes);
    if (bytes_written < 0) {
        return -1; // Erreur d'écriture
    }

    f->cursor_position += bytes_written;
    return bytes_written;
}

// Fonction de lecture depuis un fichier
int myRead(file* f, void* buffer, int nBytes) {
    if (f == NULL || buffer == NULL || nBytes <= 0) {
        return -1; // Paramètres invalides
    }

    int bytes_read = read(f->file_descriptor, buffer, nBytes);
    if (bytes_read < 0) {
        return -1; // Erreur de lecture
    }

    f->cursor_position += bytes_read;
    return bytes_read;
}

// Fonction de déplacement du curseur
void mySeek(file* f, int offset, int base) {
    if (f == NULL) {
        return; // Paramètre invalide
    }

    f->cursor_position = lseek(f->file_descriptor, offset, base);
}

// Fonction de fermeture de fichier
void myClose(file* f) {
    if (f == NULL) {
        return; // Paramètre invalide
    }

    close(f->file_descriptor);
    free(f->filename);
    free(f);
}

// Fonction de test basique
int main() {
    // Test de création de fichier
    if (myFormat("partition.txt") != 0) {
        printf("Erreur lors du formatage de la partition.\n");
        return 1;
    }

    // Test d'ouverture de fichier
    file* test_file = myOpen("test.txt");
    if (test_file == NULL) {
        printf("Erreur lors de l'ouverture du fichier.\n");
        return 1;
    }

    // Test d'écriture dans le fichier
    char buffer[20] = "Hello, world!";
    if (myWrite(test_file, buffer, strlen(buffer)) != strlen(buffer)) {
        printf("Erreur lors de l'écriture dans le fichier.\n");
        return 1;
    }

    // Test de lecture depuis le fichier
    char read_buffer[20];
    mySeek(test_file, 0, SEEK_SET);
    if (myRead(test_file, read_buffer, strlen(buffer)) != strlen(buffer)) {
        printf("Erreur lors de la lecture depuis le fichier.\n");
        return 1;
    }
    read_buffer[strlen(buffer)] = '\0'; // Ajout du caractère de fin de chaîne
    printf("Contenu du fichier : %s\n", read_buffer);

    // Fermeture du fichier
    myClose(test_file);

    return 0;
}
