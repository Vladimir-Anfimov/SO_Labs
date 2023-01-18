#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>

#define handle_error(msg) \
    { perror(msg); exit(EXIT_FAILURE);}

void copiere_rot13(char *map_addr_read, char *map_addr_write, int length);

void memory_close_maps(char *map_addr_read, char *map_addr_write, int fdIn, int fdOut, int size);

int main(int argc, char **argv)
{
    int fdIn, fdOut;
    char *map_addr_read, *map_addr_write;
    long PageSize;
    struct stat sb;

    if(argc != 3)
    {
        fprintf(stderr, "Utilizare %s [intrare-fisier] [iesire-fisier] \n", argv[0]);
        return 1;
    }

    if(0 == access(argv[2], F_OK))
    {
        printf("Fisierul %s exista deja. Doriti sa il suprascrieti? DA/NU \n", argv[2]);
        char raspuns[3];
        while(1)
        {
            scanf("%s", raspuns);
            if(strcmp(raspuns, "DA") == 0)
            {
                printf("Dreptul de suprascriere a fost acordat. \n");
                break;
            }
            else if(strcmp(raspuns, "NU") == 0)
            {
                printf("Dreptul de suprascriere NU a fost acordat. Programul se va inchide fara copiere.\n");
                return 3;
            }
        }
    }

    if( -1 == (PageSize = sysconf(_SC_PAGE_SIZE)) )
    {
        handle_error("Eroare la sysconf.");
    }   

    int flags_case = 0; // default fisiere diferite
    if(strcmp(argv[1], argv[2]) == 0) flags_case = 1;


    if( -1 == (fdIn = open(argv[1], flags_case == 0 ? O_RDONLY : O_RDWR)))
            handle_error("Eroare la deschiderea fisierului din care citim.");

    if(-1 == fstat(fdIn, &sb))
            handle_error("Eroare la fstat.");

    map_addr_read = mmap( NULL, 
                          sb.st_size, 
                          flags_case == 0 ? PROT_READ : PROT_READ | PROT_WRITE,
                          flags_case == 0 ? MAP_PRIVATE : MAP_SHARED,
                          fdIn,
                          0
                    );
    if(map_addr_read == MAP_FAILED)
        handle_error("Eroare la mapare.");

    if(flags_case == 1)
    {
        map_addr_write = map_addr_read;
        fdOut = fdIn;
    }
    else {
        if( -1 == (fdOut = open(argv[2], O_RDWR | O_CREAT, 0600)))
            handle_error("Eroare la deschiderea fisierului in care scriem.");
        map_addr_write = mmap( NULL, 
                                sb.st_size, 
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED,
                                fdOut,
                                0
                        );
        if(map_addr_write == MAP_FAILED)
            handle_error("Eroare la mapare.");
        ftruncate(fdOut, sb.st_size);
    }
    
    
    if(-1 == close(fdIn))
        handle_error("Eroare la inchiderea fisierului.");
    if(fdIn != fdOut && -1 == close(fdOut))
        handle_error("Eroare la inchiderea fisierului.");

    printf("Fisierul de intrare este: %s\n", argv[1]);
    printf("Fisierul de iesire este: %s\n", argv[2]);

    copiere_rot13(map_addr_read, map_addr_write, sb.st_size);

    if(-1 == msync(map_addr_write, sb.st_size, MS_SYNC))
        handle_error("Eroare la msync.");

   memory_close_maps(map_addr_read, map_addr_write, fdIn, fdOut, sb.st_size);

   return 0;
}


void copiere_rot13(char *map_addr_read, char *map_addr_write, int length)
{
    
    for(int i=0; i < length; i++)
    {
        if('A' <= map_addr_read[i] && map_addr_read[i] <= 'Z')
        {
            map_addr_write[i] = 65 + (map_addr_read[i] - 65 + 13) % 26;
        }
        else if('a' <= map_addr_read[i] && map_addr_read[i] <= 'z')
        {
            map_addr_write[i] = 97 + (map_addr_read[i] - 97 + 13) % 26;
        }
    }

    printf("Copierea si procesarea au avut loc cu succes.\n");
}

void memory_close_maps(char *map_addr_read, char *map_addr_write, int fdIn, int fdOut, int size)
{
    if( -1 == munmap(map_addr_read, size))
        handle_error("Eroare la munmap.");
        
    if(fdIn != fdOut &&  -1 == munmap(map_addr_write, size))
        handle_error("Eroare la munmap.");
}
