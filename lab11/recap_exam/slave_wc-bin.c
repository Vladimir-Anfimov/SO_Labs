#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#define PAGE_SIZE 4096

#define handle_error(msg) \
    {perror(msg); exit(EXIT_FAILURE);}

int main(int argc, char **argv)
{
    if(argc < 5)
        fprintf(stderr, "Format apel: ./slave file_in descriptor_out_str offset_str length_str\n"), exit(EXIT_FAILURE);

    char *file_in = argv[1];
    int descriptorWrite = atoi(argv[2]);
    int offset = atoi(argv[3]);
    int length = atoi(argv[4]);
    int descriptorRead;

    if(-1 == (descriptorRead = open(file_in, O_RDONLY)))
        handle_error("Eroare la open");

    if(-1 == lseek(descriptorRead, offset, SEEK_SET))
        handle_error("Eroare la lseek");

    int count = 0;
    while(1)
    {
        char buffer[PAGE_SIZE];
        int c_read = read(descriptorRead, buffer, PAGE_SIZE);
        if(c_read == -1) handle_error("Eroare la read");
        if(c_read == 0) break;
        for(int i=0;i<c_read && length > 0;i++)
        {
            if(isalnum(buffer[i]))
                count++;
            length--;
        }
    }

    if(sizeof(int) != write(descriptorWrite, &count, sizeof(int)))
        handle_error("Eroare la write");

    if(-1 == lseek(descriptorWrite, 0, SEEK_SET))
        handle_error("Eroare la lseek");
    close(descriptorRead);
}