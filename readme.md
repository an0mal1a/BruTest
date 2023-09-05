# BruTest

### En desarrollo...
Generador de contraseñas, utiliza un algoritmo para generar todas las combinaciones posibles.

Le falta desarrollo, actualmente puedes especificar un contraseña para conseguirla. Este programa genera todas las combinaciones
posibles, puede estar bastante tiempo en ejecucion dependiendo de la longevidad de la contraseña.

Genera millones de contraseñas en apenas segundos.
Actualmente no utiliza ningún metodo de hashing, por el momento solo acepta la contraseña en claro para comprobar con la generada.


# Parametros

        [*] REQUERIDO
                 ├──[ -t (thread)] Número de hilos
                 ├──[ -l (length)] Caracteres de la contraseña a generar
                 └──[ -p (passwd)] Actual parametro para esepcificar la contraseña a encontrar

        [!] EJEMPLOS

                (100 threads)──┐           ┌── (Password)
                    ./bruTest -t 200 -l 8 -p acK2od 
                                      └─(Long Password)   
        -------------------------------------------------------------------------------
                                             ┌──(200 threads)                                                              
                    ./bruTest -t 200 -l 8 -p acK2od 
                                                    └─(Random Length)   

# Output:

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



# Compilation

        ┌──(supervisor㉿parrot)-[~]
        └─$ gcc -g generate_all.c -o brutest -lpthread



                 
        