#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define FILE_NAME "filein.txt"

int main()
{
    int fd;

    if(0 == access(FILE_NAME, F_OK))
        remove(FILE_NAME);
    
    if(-1 == (fd = open(FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0600)))
    {
        perror("Eroare la open");
        exit(EXIT_FAILURE);
    }

    char input[20];
    scanf("%s", input);
    write(fd, input, strlen(input));
        
    close(fd);
    return 0;
}