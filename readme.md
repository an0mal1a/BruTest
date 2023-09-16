# BruTest

### En desarrollo...
Generador de contraseñas, utiliza un algoritmo para generar todas las combinaciones posibles.

Le falta desarrollo, actualmente puedes especificar hash md5 o sha256. Este programa genera todas las combinaciones
posibles, puede estar bastante tiempo en ejecucion dependiendo de la longevidad de la contraseña.

Genera millones de contraseñas en apenas segundos.
Implementado metodo MD5 de hash

# Requirements:

    sudo apt-get install libssl-dev


# Parametros

        [*] REQUERIDO
                 ├──{ -t (thread)} Número de hilos
                 ├──{ -p (passwd)} Parametro para esepcificar el hash MD5 / SHA-256
                 └──{ -l (length)} Caracteres de la contraseña a generar

        [*] Opcional
                 └──{ -d (dict)} Parametro para generar un diccionario de combinaciones        


        [!] EJEMPLOS

            {*} FIND PASSWORD "acK2od" with 200 threads and 6 chars of length

                 (200 threads)──┐           ┌── (Password)
                    ./bruTest -t 200 -l 6 -p acK2od 
                                      └─(Long Password)   
                                      
        -------------------------------------------------------------------------------

            {*} Generate Password with random length and 200 threds

                                  ┌──(200 threads)                                                              
                    ./bruTest -t 200 -l 0
                                        └─(Random Length)   

        -------------------------------------------------------------------------------        

            {*} Generate DICTIONARY with 200 threads and 8 digits of length

                Logic -> ./bruTest -w -t <threads> -l <password_len> > <name_new_dict>

                                   ┌──(Genrate Dict)                                                              
                        ./bruTest -w -t 200 -l 0 > dict
                                               └─(Random Length)   



# Normal Output:

        [*] OUTPUT:

    
                    ▄▄▄▄    ██▀███   █    ██ ▄▄▄█████▓▓█████   ██████ ▄▄▄█████▓
                    ▓█████▄ ▓██ ▒ ██▒ ██  ▓██▒▓  ██▒ ▓▒▓█   ▀ ▒██    ▒ ▓  ██▒ ▓▒
                    ▒██▒ ▄██▓██ ░▄█ ▒▓██  ▒██░▒ ▓██░ ▒░▒███   ░ ▓██▄   ▒ ▓██░ ▒░
                    ▒██░█▀  ▒██▀▀█▄  ▓▓█  ░██░░ ▓██▓ ░ ▒▓█  ▄   ▒   ██▒░ ▓██▓ ░ 
                    ░▓█  ▀█▓░██▓ ▒██▒▒▒█████▓   ▒██▒ ░ ░▒████▒▒██████▒▒  ▒██▒ ░
                    ░▒▓███▀▒░ ▒▓ ░▒▓░░▒▓▒ ▒ ▒   ▒ ░░   ░░ ▒░ ░▒ ▒▓▒ ▒ ░  ▒ ░░   
                    ▒░▒   ░   ░▒ ░ ▒░░░▒░ ░ ░     ░     ░ ░  ░░ ░▒  ░ ░    ░     
                    ░    ░   ░░   ░  ░░░ ░ ░   ░         ░   ░  ░  ░    ░      
                    ░         ░        ░                 ░  ░      ░           
                    ░                                                     
            
                        Created By An0mal1a (https://github.com/an0mal1a)
            
                                ----------------------------
                                | Password Length:   6     |
                   [♦] Details: ----------------------------
                                | Number of threads:  250  |  
                                ----------------------------
                                   PasswordToCrack: acK2od 
                
                    --------------------------------------------------------
                
                    [♦] Starting: Creating Threads and assigning priority...

                
                        [♦] Attempt: 40560624        PASSWD: acKlFB
                

                    [!] Done!  Password Found:   acK2od   |   Tries 40624264 


# Wordlist Output:

                aaaaaaaa
                aaaaaaab
                aaaaaaac
                aaaaaaad
                aaaaaaae
                aaaaaaaf
                aaaaaaag
                aaaaaaah
                ........




# Compilation

        ┌──(supervisor㉿parrot)-[~]
        └─$ gcc -g generate_all.c -o brutest -lpthread -lcrypto



                 
        