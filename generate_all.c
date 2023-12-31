#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <regex.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

#define MAX_THREADS 60
#define MIN_THREADS 1
#define MIN_PASSWORD 4

//Variables Globales
char passwordToCrack[SHA256_DIGEST_LENGTH] = "";
bool is_md5 = false;
char *mode = "attack";
const char *dict = "./dict.txt";
long tries = 0;  // Variable global para contar los intentos
long count = 0;
char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; // Cadena de caracteres que se utilizarán para generar las combinaciones

//Colores
const char *GREEN = "\033[0;32m\033[1m";
const char *end = "\033[0m\033[0m";
const char *RED = "\033[0;31m\033[1m";
const char *BLUE = "\033[0;34m\033[1m";
const char *YELLOW = "\033[0;33m\033[1m";
const char *PURLPLE = "\033[0;35m\033[1m";
const char *CYAN = "\033[0;36m\033[1m";
const char *GRAY = "\033[0;37m\033[1m";

// Prototipo de funciones
//void set_thread_priority(pthread_t thread_id, int priority);
void clean();
void hashID();
void banner();
void helpPannel(int op);
void restoreCursor();
void disapearCursor();
void ctrlCHandler(int sig);
void* init_combinations(void* arg);
void checkHashes(char* combination);
void print_md5_hash(unsigned char* md);
void print_sha256_hash(unsigned char* hash);
void print_data(int* char_of_passwd, int* threads, char* randomized);
void perform_brute(int char_of_passwd, int threads, const char* mode);
void generate_combinations(char* chars, int n_chars, char* combination, int length, int index, int start_char, int end_char, const char* mode);
bool check_hash(const char *hash_string);
bool check_args(int *char_of_passwd, int *threads, const char *hash_string, char *mode);
int setParametres(int* password_length, int* threads);


// Mutex global para proteger 'tries'
pthread_mutex_t tries_mutex = PTHREAD_MUTEX_INITIALIZER;
// Mutex global para proteger 'count
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;


// Estructura para pasar los argumentos a generate_passwords
typedef struct {
    int char_of_passwd;
    int total_threads;
    int thread_id;
    const char* mode;
} ThreadArguments;

void print_md5_hash(unsigned char* md) {
    printf("MD5 -> ");
    for(int i = 0; i < MD5_DIGEST_LENGTH; i++)
        printf("%02x", md[i]);
    printf("\n%s", RED);
}

void print_sha256_hash(unsigned char* hash) {
    printf("SHA-256 -> ");
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        printf("%02x", hash[i]);
    printf("\n");
}

void ctrlCHandler(int sig) {
    // Exit ctrl + c
    restoreCursor();
    printf("\n\n\t%s[!] %sSaliendo: Ctrl + C detectado... %s\n", RED, YELLOW, end);
    exit(1);
}

void restoreCursor(){
    system("tput cnorm");
}

void disapearCursor(){
    system("tput civis");
}

void helpPannel(int op){
    banner();
    if (op == 1){
        printf("\n\n[*] OPTIONS:\n");
        printf("\n\t -d        --> Este modo se utiliza para generar todas las combinaciones en formato de diccionario, imprimiendo todas las combinaciones.");
        printf("\n\t -t	   --> Especificar el número de hilos.");
        printf("\n\t -p        --> Especificar la contraseña en hash MD5 a crackear.");
        printf("\n\t -l	   --> Especificar el largo de la contraseña. Usa 0 para que sea aleatorio.");
        printf("\n\t -h        -->  Muestra este panel de ayuda");
        printf("\n\t--help     --> Muestra más ayuda.\n");

    } else {
        printf("\n[*] REQUERIDO\n");
        printf("\t ├──{ -t (thread) } Número de hilos");
        printf("\n\t ├──{ -p (passwd) } Contraseña en MD5");
        printf("\n\t └──{ -l (length) } Caracteres de la contraseña a generar\n");
        
        printf("\n[*] Eleccion");
        printf("\n\t └──{ -d (wrdlst) } Parametro para generar un diccionario de combinaciones");
        
        printf("\n\n\n[!] EJEMPLOS\n");
        
        printf("\n\t{*} FIND PASSWORD 'halA2k' with 60 threads and 6 chars of length\n");
        printf("\n\t\t ./bruTest -t <threads> -l <password_len> -p <md5/sha-256>\n");
        printf("\n\t\t (200 threads)──┐          ┌── (Password)");
        printf("\n\t\t    ./bruTest -t 60 -l 6 -p e0f252c678964405c8ef7d31a4e6334a ");
        printf("\n\t\t                      └─(Long Password)");
        printf("\n\n-----------------------------------------------------------------------------------------\n");
        printf("\n\t{*} Generate DICTIONARY with 60 threads and 8 digits of length\n");
        printf("\n\t\t ./bruTest -d -t <threads> -l <password_len> > <name_new_dict>\n");
        printf("\n\t     (Genrate Dict)───┐            ┌──(Send Output to File)");
        printf("\n\t\t   ./bruTest -d -t 60 -l 0 > dict");
        printf("\n\t\t                        └─(Random Length)\n");                
    }
}

void banner(){
    printf("\n\n");
    printf("%s\t\t▄▄▄▄    ██▀███   █    ██ ▄▄▄█████▓▓█████   ██████ ▄▄▄█████▓\n", GREEN);
    printf("\t\t▓█████▄ ▓██ ▒ ██▒ ██  ▓██▒▓  ██▒ ▓▒▓█   ▀ ▒██    ▒ ▓  ██▒ ▓▒\n");
    printf("\t\t▒██▒ ▄██▓██ ░▄█ ▒▓██  ▒██░▒ ▓██░ ▒░▒███   ░ ▓██▄   ▒ ▓██░ ▒░\n");
    printf("\t\t▒██░█▀  ▒██▀▀█▄  ▓▓█  ░██░░ ▓██▓ ░ ▒▓█  ▄   ▒   ██▒░ ▓██▓ ░ \n");
    printf("\t\t░▓█  ▀█▓░██▓ ▒██▒▒▒█████▓   ▒██▒ ░ ░▒████▒▒██████▒▒  ▒██▒ ░\n");
    printf("\t\t░▒▓███▀▒░ ▒▓ ░▒▓░░▒▓▒ ▒ ▒   ▒ ░░   ░░ ▒░ ░▒ ▒▓▒ ▒ ░  ▒ ░░   \n");
    printf("\t\t▒░▒   ░   ░▒ ░ ▒░░░▒░ ░ ░     ░     ░ ░  ░░ ░▒  ░ ░    ░     \n");
    printf("\t\t ░    ░   ░░   ░  ░░░ ░ ░   ░         ░   ░  ░  ░    ░      \n");
    printf("\t\t ░         ░        ░                 ░  ░      ░           \n");
    printf("\t\t ░                                                     %s\n\n", end);

    printf("\t\t\t Created By An0mal1a (https://github.com/an0mal1a)\n\n");


}

void* init_combinations(void* arg) {
    ThreadArguments* args = (ThreadArguments*)arg;
    int char_of_passwd = args->char_of_passwd;
    int total_threads = args->total_threads;
    int thread_id = args->thread_id;
    const char* mode = args->mode;
    int n_chars = strlen(chars);

    // Calcula el rango de caracteres que este hilo generará
    int chars_per_thread = n_chars / total_threads;
    int start_char = thread_id * chars_per_thread;
    int end_char = (thread_id == total_threads - 1) ? n_chars : (start_char + chars_per_thread);

    if (char_of_passwd == 0){
        char_of_passwd = (rand() % 13) + 8; 
    }

    // Prepara un buffer para generar contraseñas
    char combination[char_of_passwd + 1];
    combination[char_of_passwd] = '\0';
    
    // Comienza a generar contraseñas
    generate_combinations(chars, n_chars, combination, char_of_passwd, 0, start_char, end_char, mode);
    return NULL;
}

void generate_combinations(char* chars, int n_chars, char* combination, int length, int index, int start_char, int end_char, const char* mode) {
    
    // Cuando hemos generado una contraseña completa, imprímela
    if (index == length) {

        // Bloquea el mutex antes de modificar 'tries'
        pthread_mutex_lock(&tries_mutex); 

        // Incrementa 'tries' dentro de la sección crítica protegida por el mutex
        tries++;
        
        // Desbloquea el mutex después de modificar 'tries'
        pthread_mutex_unlock(&tries_mutex);
         
        // Bloquea el mutex antes de modificar 'count'
        pthread_mutex_lock(&count_mutex);
        
        // Incrementa 'count' dentro de la sección crítica protegida por el mutex
        count++;

        // Desbloquea el mutex después de modificar 'count'
        pthread_mutex_unlock(&count_mutex);

        // Abre el archivo en modo appen ("a" añadira contenido)
        if(strcmp(mode, "dict") == 0){
            printf("%s\n", combination);
            /*FILE* file = fopen(dict, "a");
            if (file){
                fprintf(file, "%s\n", combination);
                fclose(file);

            } else {
                printf("Error al abrir el archivo para escritura.\n");
                fclose(file);
                exit(1);
            }*/

        } else if (strcmp(mode, "attack") == 0){
        
            if (count > 80000){
                printf("\t\t%s[♦] Attempt:%s %ld        %sPASSWD: %s%s%s\r", YELLOW, CYAN, tries, YELLOW, CYAN, combination, end);
                //printf("\t\t[♦] Attempt: %ld       PASSWD: %s\r", tries, combination);
                count = 0;
            } 

            checkHashes(combination);
            
        } else if(strcmp(mode, "wordlist") == 0){
            //readWordlist();
        }

        
    
        // Manual check for the password  
        /*if (strcmp(combination, "hala1") == 0){
            printf("\n\n\n\t%s[*]%s Done!%s  Password Found:  %s %s %s  |  %s Tries %s%ld %s\n", YELLOW, GRAY, BLUE, YELLOW, combination, RED, BLUE, YELLOW, tries, RED);
            restoreCursor();
            exit(0);

        }*/

        return;
    }
        
    
    for (int i = start_char; i < end_char; i++) {
        combination[index] = chars[i];
        generate_combinations(chars, n_chars, combination, length, index + 1, 0, n_chars, mode);
    }
}

void checkHashes(char* combination){
    // Usar MD5
    if (is_md5) {
        unsigned char digest[MD5_DIGEST_LENGTH];
        MD5((unsigned char*)combination, strlen(combination), digest);
        if (memcmp(digest, passwordToCrack, MD5_DIGEST_LENGTH) == 0) {
            printf("\n\n\n\t%s[!]%s Done!%s  Password Found:  %s %s %s  |  %s Tries %s%ld %s\n", YELLOW, GRAY, BLUE, YELLOW, combination, RED, BLUE, YELLOW, tries, RED);
            restoreCursor();
            exit(0);
        }
    // Usar SHA-256
    } else { 
        
        unsigned char digest[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)combination, strlen(combination), digest);
        if (memcmp(digest, passwordToCrack, SHA256_DIGEST_LENGTH) == 0) {
            printf("\n\n\n\t%s[!]%s Done!%s  Password Found:  %s %s %s  |  %s Tries %s%ld %s\n", YELLOW, GRAY, BLUE, YELLOW, combination, RED, BLUE, YELLOW, tries, RED);
            restoreCursor();
            exit(0);
        }
    }  
    return;
}

void perform_brute(int char_of_passwd, int threads, const char* mode){
    pthread_t thread_ids[MAX_THREADS];
    ThreadArguments thread_args[MAX_THREADS];

    if (strcmp(mode, "dict") != 0)
        printf("\n\t%s[♦]%s Starting: %sCreating Threads and assigning priority... %s\n\n\n", YELLOW, GRAY, BLUE, end);
        

    disapearCursor();


    // Bucle que crea los hilos
    for (int i = 0; i < threads; i++) {
        thread_args[i].char_of_passwd = char_of_passwd;
        thread_args[i].total_threads = threads;
        thread_args[i].thread_id = i;
        thread_args[i].mode = mode;

        // Crear los hilos de ejecución
        pthread_create(&thread_ids[i], NULL, init_combinations, (void*)&thread_args[i]);
    }

    // Esperar a que todos los hilos terminen
    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    printf("\n\n\n%s[!]%s Ending: %sAll Posibilities are tired... %s(%ld)\n\n\n\n", YELLOW, GRAY, BLUE, end, tries);  
    
    // Mostrar cursor
    restoreCursor();

    return;
}

void clean(){
    // Comprobación de si estamos en Windows
    #ifdef _WIN32 
        system("cls"); // Limpia la pantalla en Windows
    #else
        system("clear"); // Limpia la pantalla en Unix/Linux
    #endif
    return;
}

int check_file(const char* file) {
    bool exist = access(file, F_OK) != -1;
    if (exist) {
        return 0;
    } else {
        printf("\n\t%s[!] %sError: El archivo no existe... \t%s %s\n", RED, YELLOW, end, file);
        return 1;
    }
}

void print_data(int* char_of_passwd, int* threads, char* randomized){
    // Limpiamos pantalla
    clean();
    //Mostramos banner
    banner();
    //Mostramos informacion seleccionada
    printf("\n\n\n\t\t     %s----------------------------%s\n", RED, end);

    if (*char_of_passwd == 0)
        printf("\t\t     %s| %sPassword Length:  %s %s\t%s   |\n", RED, BLUE, YELLOW, randomized, RED);
            
    else 
        printf("\t\t     %s| %sPassword Length:  %s %d\t%s |\n", RED, BLUE, YELLOW, *char_of_passwd, RED);
    
    printf("\t%s[♦]%s Details: %s----------------------------%s\n", YELLOW, GRAY, RED);
    printf("\t\t     %s| %sNumber of threads: %s%d\t  %s |  \n", RED, BLUE, YELLOW, *threads, RED);
    printf("\t\t     ----------------------------%s\n", end);

    // Si se ha seleccionado un contraseña a crackear, lo mostramos 
    if (strcmp(passwordToCrack, "") != 0)
        printf("\t\t       %sPasswordToCrack: %s ", BLUE, YELLOW);
        if (is_md5){
            print_md5_hash(passwordToCrack);
        } else {
            print_sha256_hash(passwordToCrack);
        }
        printf("\n\t--------------------------------------------------------%s\n", end);
    
    
}


bool check_args(int *char_of_passwd, int *threads, const char *hash_string, char *mode) {
    int things = 0;
    char randomized[] = "Rand";
    

    // Comprobación del hash
    if (strcmp(mode, "attack") == 0 && hash_string != NULL){
        if (!check_hash(hash_string)) 
            return false;
    } else if (strcmp(mode, "attack") == 0 && hash_string == NULL) {
        printf("\n\t[!] ERROR: Hash no especificado...\n");
        exit(1);
    }

   

    //Mensaje de error: Demasiados hilos
    if (*threads > MAX_THREADS) {
        printf("\n\t%s[!] %sError: Demasiados hilos... (MAX -> 60) \n%s", RED, YELLOW, end);
    } else {
        if (*threads > MIN_THREADS) {
            ++things;
        } else {
            printf("\n\t%s[!] %sError: Muy pocos hilos... (MIN -> 1)\n%s", RED, YELLOW, end);
        }
    }

    // Comprobacion de la contraseña
    if (*char_of_passwd <= MIN_PASSWORD) {
        // Random length
        if (*char_of_passwd == 0) {
            ++things;
        } else { //Mensaje de error: Contraseña muy corta
            printf("\n\t%s[!] %sError: La contraseña no cumple los requsitmos minimos (5)%s\n", RED, YELLOW, end);
        }
    } else {
        ++things;
    }

    //things = things + check_file(file);

    if (things == 2 && strcmp(mode, "attack") == 0) {
        print_data(char_of_passwd, threads, randomized);
        return true;
    
    } else if (strcmp(mode, "dict") == 0){
        return true;

    } else {
        exit(0); // Salimos, argumentos incorrectos...
    }

    return false;
}

int addPasswordToFound(){
    char set[10] = "";

    printf("\n\n%s[¿?]%s Details:%s  Do you want to set a password to found?:  [ y / (N > default) ]%s  ", YELLOW, GRAY, end, YELLOW, end);
    scanf("%s", set);

    if (strlen(set) > 1)
        return 1;
    else
        set[0] = tolower(set[0]);

    if (strcmp(set, "y") == 0){
        printf("\n\t%s[*]%s Required:%s  Set password to found:%s  ", YELLOW, GRAY, end, YELLOW, end);
        scanf("%99s", passwordToCrack);
        return 0;
    } else { return 1; }

    return 0;
}

bool check_hash(const char *hash_string) {
    if (strlen(hash_string) == 32) {
        is_md5 = true;

        // Mirar si se está bien escrito
        for (int j = 0; j < 32; j++) {
            if (!((hash_string[j] >= '0' && hash_string[j] <= '9') || (hash_string[j] >= 'a' && hash_string[j] <= 'f'))) {
                printf("Error: el hash debe ser una cadena de 32 (MD5) caracteres hexadecimales.\n");
                return false;
            }
        }
    } else if (strlen(hash_string) == 64) {
        is_md5 = false;

        // Mirar si se está bien escrito
        for (int j = 0; j < 64; j++) {
            if (!((hash_string[j] >= '0' && hash_string[j] <= '9') || (hash_string[j] >= 'a' && hash_string[j] <= 'f'))) {
                printf("Error: el hash debe ser una cadena 64 (SHA-256) caracteres hexadecimales.\n");
                return false;
            }
        }
    } else {
        printf("Error: el hash debe ser una cadena de 32 (MD5) o 64 (SHA-256) caracteres hexadecimales.\n");
        return false;
    }

    // Convertir la cadena hexadecimal al formato adecuado (MD5 o SHA-256)
    if (is_md5) {
        for (int j = 0; j < MD5_DIGEST_LENGTH; j++) {
            sscanf(&hash_string[j * 2], "%02hhx", &passwordToCrack[j]);
        }
    } else {
        for (int j = 0; j < SHA256_DIGEST_LENGTH; j++) {
            sscanf(&hash_string[j * 2], "%02hhx", &passwordToCrack[j]);
        }
    }

    return true;
}

void hashID(){
    if (strcmp(passwordToCrack, "") != 0){
        char command[300];
        snprintf(command, sizeof(command), "name-that-hash --text %s --no-banner --no-john --no-hashcat -g | grep name | head -n 1 | awk '{print $2}' | tr -d '\"' | tr -d ','", passwordToCrack);

        FILE* fp = popen(command, "r");
        if (fp == NULL) {
            fprintf(stderr, "\n\nError occurred while try to check hash\n");
            exit(1);
        }

        printf("%s", fp);
    }  
}

int main(int argc, char *argv[]) {
    //Variables a comprobar
    int threads = 20;
    int password_length = 12;
    const char *hash_string = NULL;
    
    // Atrapamos ctrl + c 
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
        perror("Error al configurar el manejador de señales");
        exit(EXIT_FAILURE);
        restoreCursor();
    }

    // Print Banner
    //banner();

    // Si existe mas de 1 argumento, analiza los argumentos de línea de comandos
    if (argc > 1) {

        for (int i = 1; i < argc; i++) {
            // Wordlist mode
            if (strcmp(argv[i], "-h") == 0) {
                helpPannel(1);
                exit(0);
                
            } else if (strcmp(argv[i], "--help") == 0) {
                helpPannel(0);
                exit(0);
            }

            if (strcmp(argv[i], "-d") == 0) {
                mode = "dict";

                // Thread arguments
            } if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
                threads = atoi(argv[i + 1]);

                // Long Password
            } if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
                password_length = atoi(argv[i + 1]);

                // Set password to found
            } if (strcmp(argv[i], "-p") == 0 && i + 1 < argc && strcmp(mode, "attack") == 0) {
                hash_string = argv[i + 1];
            }

        }
        if (!check_args(&password_length, &threads, hash_string, mode)) {
            return 1;
        }
    } else {
        helpPannel(1);
        exit(1);
    }

    // Miramos si se trata de un hash:
    //check_hash();

    //Iniciamos fuerza bruta
    perform_brute(password_length, threads, mode);

    // Mostrar cursor
    restoreCursor();
    return 0;
}