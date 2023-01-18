#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

#define MAX_SIZE_LINE 255

#define handle_error(msg) \
    {perror(msg); exit(EXIT_FAILURE);}

pid_t Fork()
{   
    pid_t pid;
    if(-1 == (pid = fork())) handle_error("Eroare la fork");
    return pid;
}

void Pipe(int p[])
{
    if(-1 == pipe(p)) handle_error("Eroare la pipe");
}

void Close(int descriptor)
{
    if(-1 == close(descriptor)) handle_error("Eroare la inchiderea descriptorului");
}

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        fprintf(stderr, "Apel: %s descriptor-pipe\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int fd = atoi(argv[1]);

    char buffer[MAX_SIZE_LINE];

    int fifo;
    if(-1 == (fifo = open("fifo_P1toP2", O_WRONLY)))
        handle_error("Eroare la open file in WR");

    while(1)
    {
        int size;
        int c_read = read(fd, &size, sizeof(int));
        if(c_read == -1) handle_error("Eroare la citirea din pipe");
        if(c_read == 0) break;
        
        c_read = read(fd, buffer, size);
        if(c_read == -1) handle_error("Eroare la citirea din pipe");

        int modificate = 0;
        for(int i=0;i<size;i++)
        {
            if(isupper(buffer[i]))
            {
                modificate++;
                buffer[i] = tolower(buffer[i]);
            }
        }

        if(sizeof(int) != write(fifo, &size, sizeof(int)))
            handle_error("Eorare la write in fifo");
            
        if(sizeof(int) != write(fifo, &modificate, sizeof(int)))
            handle_error("Eorare la write in fifo");
        
        if(size != write(fifo, buffer, size))
            handle_error("Eorare la write in fifo");
    }

    Close(fifo);
}