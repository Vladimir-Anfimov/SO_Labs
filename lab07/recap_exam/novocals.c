#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define handle_error(msg) \
    {fprintf(stderr, msg); fprintf(stderr, "\n"); exit(EXIT_FAILURE);}

int main(int argc, char **argv)
{
    int fdIn, fdOut;
    char file_in[PATH_MAX], file_out[PATH_MAX];
    long pageSize;

    if(-1 == (pageSize = sysconf(_SC_PAGE_SIZE)))
        handle_error("Eroare la sysconf");

    if(argc < 2)
        handle_error("Format apel: ./novocals fisier1 [fisier2]");
    if(argc == 2 || 0 == access(argv[2], F_OK))
    {
        char raspuns[3];
        printf("Acordati permisiunea de suprascriere a fisierului %s ? (DA/NU)\n", argc == 2 ? argv[1] : argv[2]);
        while(1)
        {
            scanf("%s", raspuns);
            if(strcmp(raspuns, "DA") == 0) 
            {
                printf("Permisiunea a fost acordata.\n");
                strcpy(file_in, argv[1]);
                strcpy(file_out, argc == 2 ? argv[1] : argv[2]);
                break;
            }
            else if(strcmp(raspuns, "NU") == 0)
                handle_error("Permisiunea nu a fost acordata. Programul se va inchide.");
        }
    }
    else {
        strcpy(file_in,  argv[1]);
        strcpy(file_out, argv[2]);
    }

    if(-1 == (fdIn = open(file_in, O_RDONLY)))
        handle_error("Eroare la open file_in");
    if(-1 == (fdOut = open(file_out, O_WRONLY | O_CREAT | O_TRUNC, 0600)))
        handle_error("Eroare la open file_out");
    
    int total_bytes = 0;
    while(1)
    {
        char buffer[pageSize];
        ssize_t c_read = read(fdIn, buffer, pageSize);
        if(c_read == -1)
            handle_error("Eroare la citirea din fisier");
        if(c_read == 0) break;
        for(int i = 0; i < c_read; i++)
        {
            if(strchr("AEIOUaeiou", buffer[i]) == NULL)
            {
                if(sizeof(char) != write(fdOut, &buffer[i], sizeof(char)))
                    handle_error("Eroare la write in fisier");
                total_bytes++;    
            }
        }
    }

    ftruncate(fdOut, total_bytes);
    close(fdIn);
    close(fdOut);



        
}