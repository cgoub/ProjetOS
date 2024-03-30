#define BLOCK_SIZE 2048 // Taille de bloc 
#define PARTITION_SIZE 32 * 1024 * 1024 // Taille de la partition


/**
 * @brief Structure "file" réalisé pour l'implémentation de notre bibliothèque.
 * 
 */
typedef struct {
    char nom[20]; /**< Nom du fichier */
    int position; /**< position de la tête de lecture */
    int dispo; /**< Disponibilité du block */
    int debut; /**< position du début du fichier */
    int taille; /**< taille du fichier */
} file;

/* Prototypes des fonctions */
int myFormat(char* partitionName);
file* myOpen(char* fileName);
void visualisation(char* partitionName);
void mySeek(file* f, int offset, int base);
int myRead(file* f, void* buffer, int nBytes);
int myWrite(file* f, void* buffer, int nBytes);
int size(file* f);