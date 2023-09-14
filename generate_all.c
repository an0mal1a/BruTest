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

#define MAX_THREADS 60
#define MIN_THREADS 1
#define MAX_PASSWORD_CHARS 33
#define MIN_PASSWORD 4

//Variables Globales
char passwordToCrack[MAX_PASSWORD_CHARS] = "";
//const char* target_network = "MOVISTAR_574C";
//const char* file = "/root/EvilHunter_Data/captures/MOVISTAR_574C/capture-01.cap"; 
bool hash = false;
// Cadena de caracteres que se utilizarán para generar las combinaciones  ABCDEFGHIJKLMNOPQRSTUVWXYZ
char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

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
void banner();
void check_hash();
void helpPannel(int op);
void restoreCursor();
void disapearCursor();
void ctrlCHandler(int sig);
void* init_combinations(void* arg);
void print_data(int* char_of_passwd, int* threads, char* randomized);
void perform_brute(int char_of_passwd, int threads, const char* mode);
void generate_combinations(char* chars, int n_chars, char* combination, int length, int index, int start_char, int end_char, const char* mode);
bool check_args(int* char_of_passwd, int* threads);
int setParametres(int* password_length, int* threads);

// Variable global para contar los intentos
long tries = 0;
long count = 0;

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
    int i;
    for(i = 0; i < MD5_DIGEST_LENGTH; i++)
        printf("%02x", md[i]);
    printf("\n%s", RED);
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
    
    if (op == 1){
        banner();
        printf("\n\n[*] OPTIONS:\n");
        printf("\n\t -w        --> Este modo se utiliza para generar todas las combinaciones en formato de diccionario, imprimiendo todas las combinaciones.");
        printf("\n\t -t	   --> Especificar el número de hilos.");
        printf("\n\t -p        --> Especificar la contraseña en hash MD5 a crackear.");
        printf("\n\t -l	   --> Especificar el largo de la contraseña. Usa 0 para que sea aleatorio.");
        printf("\n\t -h        -->  Muestra este panel de ayuda");
        printf("\n\t--help     --> Muestra más ayuda.\n");

    } else {
        printf("\n[*] REQUERIDO\n");
        printf("\t ├──{ -t (thread) } Número de hilos");
        printf("\n\t └──{ -l (length) } Caracteres de la contraseña a generar\n");
        
        printf("\n[*] Eleccion");
        printf("\n\t ├──{ -p (passwd) } Contraseña en MD5");
        printf("\n\t └──{ -w (wrdlst) } Parametro para generar un diccionario de combinaciones");
        
        printf("\n\n\n[!] EJEMPLOS\n");
        
        printf("\n\t{*} FIND PASSWORD 'acK2od' with 200 threads and 6 chars of length");
        printf("\n\t\t(200 threads)──┐           ┌── (Password)");
        printf("\n\t\t    ./bruTest -t 200 -l 6 -p acK2od ");
        printf("\n\t\t                      └─(Long Password)");
        printf("\n\n-------------------------------------------------------------------------------\n");
        printf("\n\t{*} Generate Password with random length and 200 threds\n");
        printf("\n\t\t            ┌──(200 threads)");
        printf("\n\t\t./bruTest -t 200 -l 0");
        printf("\n\t\t                   └─(Random Length)");
        printf("\n\n-------------------------------------------------------------------------------\n");
        printf("\n\t{*} Generate DICTIONARY with 200 threads and 8 digits of length\n");
        printf("\n\t\tLogic -> ./bruTest -w -t <threads> -l <password_len> > <name_new_dict>");
        printf("\n\t\t             ┌──(Genrate Dict");
        printf("\n\t\t  ./bruTest -w -t 200 -l 0 > dict");
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

        //printf("%ld\n\n", count);
        if(strcmp(mode, "wordlist") == 0){
            printf("\n%s", combination);

        } else if (count > 65000 && strcmp(mode, "attack") == 0){
            printf("\t\t%s[♦] Attempt:%s %ld        %sPASSWD: %s%s%s\r", YELLOW, CYAN, tries, YELLOW, CYAN, combination, end);
            //printf("\t\t[♦] Attempt: %ld       PASSWD: %s\r", tries, combination);
            count = 0;
        }
        
        unsigned char digest[MD5_DIGEST_LENGTH];
        MD5((unsigned char*)combination, strlen(combination), (unsigned char*)&digest);
  
        // Compara el hash generado con el hash introducido
        if(memcmp(digest, passwordToCrack, MD5_DIGEST_LENGTH) == 0) {
            printf("\n\n\n\t%s[!]%s Done!%s  Password Found:  %s %s %s  |  %s Tries %s%ld %s\n", YELLOW, GRAY, BLUE, YELLOW, combination, RED, BLUE, YELLOW, tries, RED);
            restoreCursor();
            exit(0);
        } 

        // Manual check for the password  
        /*if (strcmp(combination, "acK12") == 0){
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

void perform_brute(int char_of_passwd, int threads, const char* mode){
    pthread_t thread_ids[MAX_THREADS];
    ThreadArguments thread_args[MAX_THREADS];

    printf("\t%s[♦]%s Starting: %sCreating Threads and assigning priority... %s\n\n\n", YELLOW, GRAY, BLUE, end);
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
        printf("\t\t     %s| %sPassword Length:  %s %d\t%s   |\n", RED, BLUE, YELLOW, *char_of_passwd, RED);
    
    printf("\t%s[♦]%s Details: %s----------------------------%s\n", YELLOW, GRAY, RED);
    printf("\t\t     %s| %sNumber of threads: %s%d\t  %s |  \n", RED, BLUE, YELLOW, *threads, RED);
    printf("\t\t     ----------------------------%s\n", end);

    // Si se ha seleccionado un contraseña a crackear, lo mostramos 
    if (strcmp(passwordToCrack, "") != 0)
        printf("\t\t       %sPasswordToCrack: %s ", BLUE, YELLOW);
        print_md5_hash(passwordToCrack);
        printf("\n\t--------------------------------------------------------%s\n", end);
    
    printf("%s\n", end);
}


bool check_args(int* char_of_passwd, int* threads){
    int things = 0;
    char randomized[] = "Rand";

    //Mensaje de error: Demasiados hilos
    if (*threads > MAX_THREADS) {
        printf("\n\t%s[!] %sError: Demasiados hilos... (MAX -> 60) \n%s", RED, YELLOW, end);
        
    } else { 
        if (*threads > MIN_THREADS){
            ++things; 
        } else { printf("\n\t%s[!] %sError: Muy pocos hilos... (MIN -> 1)\n%s", RED, YELLOW, end); }
    }

    // Comprobacion de la contraseña
    if (*char_of_passwd <= MIN_PASSWORD){
        
        // Random length
        if(*char_of_passwd == 0){
            ++things;

        } else { //Mensaje de error: Contraseña muy corta
            printf("\n\t%s[!] %sError: La contraseña no cumple los requsitmos minimos (5)%s\n", RED, YELLOW, end);
            }
        
    } else { ++things; }
    
    //things = things + check_file(file);

    if (things == 2){     
        
        print_data(char_of_passwd, threads, randomized);
        return true;

      // salimos, argumenos incorrectos...  
    } else { exit(0); }

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

int setParametres(int* password_length, int* threads){
    // Escribimos Password Length
    printf("\n\n\n%s[*]%s Required:%s  Set password Length:%s  ", YELLOW, GRAY, end, YELLOW, end);
    scanf("%d", password_length);
    
    // Separador
    printf("--------------------------------------------------------%s", end);
    
    // Escribimos Numero de hilos
    printf("%s\n[*]%s Required:%s  Set number of threads:%s  ", YELLOW, GRAY, end, YELLOW, end);
    scanf("%d", threads);

    // Separador
    printf("--------------------------------------------------------%s", end);
    addPasswordToFound();
    disapearCursor();

    // Comprobamos argumentos
    check_args(password_length, threads);

    return 0;

}

void check_hash(){
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

int main(int argc, char* argv[]) {
    //Variables a comprobar
    int threads = 20; 
    int password_length  = 0;
    const char* mode = "attack";
    // Atrapamos ctrl + c 
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
        perror("Error al configurar el manejador de señales");
        exit(EXIT_FAILURE);
        restoreCursor();
    }

    // Print Banner
    //banner();

    // Si existe mas de 1 argumento, analiza los argumentos de línea de comandos
    if (argc > 1){
        
        for (int i = 1; i < argc; i++) {
            // Wordlist mode
            if(strcmp(argv[i], "-h") == 0){
                helpPannel(1);
                exit(0);
            } else if(strcmp(argv[i], "--help") == 0){
                helpPannel(0);
                exit(0);
            }

            if(strcmp(argv[i], "-w") == 0){
                mode = "wordlist";
                
            // Thread arguments
            } if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
                threads = atoi(argv[i + 1]);

                // Long Password
            } if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
                password_length = atoi(argv[i + 1]);

                // Set password to found
            } if (strcmp(argv[i], "-p") == 0 && i + 1 < argc && strcmp(mode, "attack") == 0){
                if(strlen(argv[i + 1]) != 32) {
                    printf("Error: el hash debe ser una cadena de 32 caracteres hexadecimales.\n");
                    return 1;
                }
                for(int j = 0; j < 32; j++) {
                    if(!((argv[i + 1][j] >= '0' && argv[i + 1][j] <= '9') || (argv[i + 1][j] >= 'a' && argv[i + 1][j] <= 'f'))) {
                        printf("Error: el hash debe ser una cadena de 32 caracteres hexadecimales.\n");
                        return 1;
                    }
                }
                for(int j = 0; j < MD5_DIGEST_LENGTH; j++) {
                    sscanf(&argv[i + 1][j*2], "%2hhx", &passwordToCrack[j]);
                }
            }
        }
        check_args(&password_length, &threads);
    } 

    // Especifiamos paremetros
    else { setParametres(&password_length, &threads); }


    // Miramos si se trata de un hash:
    //check_hash();
    

    //Iniciamos fuerza bruta
    perform_brute(password_length, threads, mode);
    
    // Mostrar cursor
    restoreCursor();
    return 0;
}
