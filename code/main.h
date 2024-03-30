#define BLOCK_SIZE 2048 // Taille de bloc 
#define PARTITION_SIZE 32 * 1024 * 1024 // Taille de la partition


// Structure de fichier
typedef struct {
    char nom[20];
    int position;
    int dispo;
    int debut;
    int taille;
} file;

/* Prototypes des fonctions */
int myFormat(char* partitionName);
file* myOpen(char* fileName);
void visualisation(char* partitionName);
void mySeek(file* f, int offset, int base);
int myRead(file* f, void* buffer, int nBytes);
int myWrite(file* f, void* buffer, int nBytes);
int size(file* f);