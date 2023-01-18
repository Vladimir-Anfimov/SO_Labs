#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#define FILE_NAME "filein.txt"

int main()
{
    int fd;
    
    if(0 == access(FILE_NAME, F_OK))
        remove(FILE_NAME);

    if(-1 == (fd = open(FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0600)))
    {
        perror("Eroare la open");
        exit(1);
    }

    for(int number=1;number<=100;number++)
    {
        if(-1 == write(fd, &number, sizeof(int)))
        {
            perror("Eoare la write");
            exit(2);
        }
    }

    close(fd);
}