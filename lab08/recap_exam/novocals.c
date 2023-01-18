#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define handle_error(msg) \
    {perror(msg); exit(EXIT_FAILURE);}

int main(int argc, char **argv)
{
    int fdIn, fdOut;
    char file_in[PATH_MAX], file_out[PATH_MAX];
    long pageSize;
    char *map_addr_in, *map_addr_out;
    struct stat sb;

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
    if(-1 == (fdOut = open(file_out, O_RDWR | O_CREAT | O_TRUNC, 0600)))
        handle_error("Eroare la open file_out");

    if(-1 == fstat(fdIn, &sb))
        handle_error("Eroare la fstat");
    if(-1 == ftruncate(fdOut, sb.st_size))
        handle_error("Eroare la ftruncate");

    if(strcmp(file_in, file_out) == 0)
    {
        map_addr_in = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fdOut, 0);
        map_addr_out = map_addr_in;
    }
    else {
        map_addr_in = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fdIn, 0);
        map_addr_out = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fdOut, 0);
    }
    
    if(map_addr_in == MAP_FAILED)
        handle_error("Eroare la mapare map_addr_in");
    if(map_addr_out == MAP_FAILED)
        handle_error("Eroare la mapare map_addr_out");
    
    int total_bytes = 0;

    for(int i=0;i<sb.st_size;i++)
    {
        if(strchr("AEIOUaeiou", map_addr_in[i]) == 0)
            map_addr_out[total_bytes++] = map_addr_in[i];
    }

    if(-1 == msync(map_addr_out, sb.st_size, MS_SYNC))
        handle_error("Eroare la msync");
    if(-1 == munmap(map_addr_in, sb.st_size))
        handle_error("Eroare la munmap map_addr_in");
    if(-1 == munmap(map_addr_out, sb.st_size))
        handle_error("Eroare la munmap map_addr_out");

    if(-1 == ftruncate(fdOut, total_bytes))
        handle_error("Eroare la ftruncate");
    if(-1 == close(fdIn))
        handle_error("Eroare la inchiderea descriptorului fdIn");
    if(-1 == close(fdOut))
        handle_error("Eroare la inchiderea descriptorului fdOut");
        
}