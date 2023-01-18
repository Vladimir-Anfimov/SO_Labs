#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define handle_error(msg) \
    {perror(msg); exit(EXIT_FAILURE);}

int main(int argc, char **argv)
{
    int fdIn, fdOut;
    long PageSize;
    int *map_addr_in, *map_addr_out;
    struct stat sb;

    if(argc < 3)
        handle_error("Format apel: ./artmean fisier-in fisier-out");
    
    if(-1 == (PageSize = sysconf(_SC_PAGE_SIZE)))
        handle_error("Eroare la sysconf");
    
    if(-1 == (fdIn = open(argv[1], O_RDONLY)))
        handle_error("Eroare la open citire");
    
    if(-1 == (fdOut = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0600)))
        handle_error("Eroare la open citire");
    
    if(-1 == fstat(fdIn, &sb))
        handle_error("Eroare la fstat");

    if(MAP_FAILED == (map_addr_in = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fdIn, 0)))
        handle_error("Eroare la mmap read");
    
    /// !!!!!!!!!!
    if(MAP_FAILED == (map_addr_out = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fdOut, 0)))
        handle_error("Eroare la mmap write");
    
    int line = 0, m = 0, a = 0;
    for(int i = 0; i < sb.st_size / sizeof(int); i++)
    {
        printf("%d\n", map_addr_in[i]);
    }
    
}