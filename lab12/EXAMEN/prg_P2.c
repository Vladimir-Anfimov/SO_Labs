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

    int descriptorWrite = atoi(argv[1]);

    int fifo;
    if(-1 == (fifo = open("fifo_P1toP2", O_RDONLY)))
        handle_error("Eroare la open file in RD");

    char buffer[MAX_SIZE_LINE];
    int size, modificate;
    int precedenta = -1;
    while(1)
    {
        ssize_t c_read = read(fifo, &size, sizeof(int));
        if(c_read == -1) handle_error("Eroare la citirea din fifo");
        if(c_read == 0) break;

        c_read = read(fifo, &modificate, sizeof(int));
        if(c_read == -1) handle_error("Eroare la citirea din fifo");

        c_read = read(fifo, buffer, size);
        if(c_read == -1) handle_error("Eroare la citirea din fifo");
    
        for(int i=0;i<size;i++)
        {
            modificate += buffer[i];
        }

        if(precedenta == -1){
            precedenta = modificate;
        }
        else {
            if(precedenta != modificate)
            {
              if(sizeof(int) != write(descriptorWrite, &modificate, sizeof(int)))
                handle_error("Eroare la write in pipe");
              precedenta = modificate;
            }
        }
    }

    Close(descriptorWrite);
    Close(fifo);
    return 0;
}