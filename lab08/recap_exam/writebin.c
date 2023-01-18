#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd, numberInput;
    
    if(-1 == (fd = open("filein.txt", O_WRONLY | O_CREAT | O_TRUNC)))
    {
        perror("Eroare la open");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        scanf("%d", &numberInput);
        if(numberInput == -1) break;
        if(sizeof(numberInput) != write(fd, &numberInput, sizeof(numberInput)))
        {
            perror("Eroare la scriere");
            exit(EXIT_FAILURE);
        }
    }
    close(fd);
    return 0;
}