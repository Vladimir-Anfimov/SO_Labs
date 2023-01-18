#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

struct Pair {
    int key, value;
};

int main(int argc, char **argv)
{
    
    if(argc != 3)
    {
        fprintf(stderr, "Format apel: [fisier.bin] [numar_perechi]\n");
        return 0;
    }

    int count = atoi(argv[2]), fd;
    
    if(-1 == (fd = open(argv[1], O_WRONLY)))
    {
        perror("Eroare la deschiderea fisierului: ");
        return 0;
    }

    struct Pair pair;
    
    for(int i=1;i<=count;i++)
    {
        printf("Scrie perechea %d: ", i);
        pair.key = i;
        pair.value = 0;
        if(-1 == write(fd, &pair, sizeof(struct Pair)))
        {
            perror("\nEroare la scrierea in fisier: ");
            return 0;
        }
    }

    close(fd);
    return 0;
}