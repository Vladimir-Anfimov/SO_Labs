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
    char cale_fisier[PATH_MAX];
    int fd, PageSize;
    int pipe0to1[2], pipe2to0[2];

    if(argc < 2)
    {
        fprintf(stderr, "Apel: %s cale-fisier\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(-1 == mkfifo("fifo_P1toP2", 0600) && errno != EEXIST)
        handle_error("Eroare la mkfifo");

    strcpy(cale_fisier, argv[1]);

    Pipe(pipe0to1);

    if(0==Fork())
    {
        Close(pipe0to1[1]);
        char descriptorRead[30];
        sprintf(descriptorRead, "%d", pipe0to1[0]);
        execl("P1", "P1", descriptorRead, NULL);
        handle_error("Eroare la execl P1");
        
    }

    Close(pipe0to1[0]);

    Pipe(pipe2to0);

    if(0 == Fork())
    {
        Close(pipe2to0[0]);
        char descriptorWrite[30];
        sprintf(descriptorWrite, "%d", pipe2to0[1]);
        execl("P2", "P2", descriptorWrite, NULL);
        handle_error("Eroare la execl P2");
    }
    
    Close(pipe2to0[1]);
    
    if(-1 == (fd = open(cale_fisier, O_RDONLY)))
        handle_error("Eroare la open file");

    if(-1 == (PageSize = sysconf(_SC_PAGE_SIZE)))
        handle_error("Eroare la sysconf");

    char line[MAX_SIZE_LINE];
    char *buffer = malloc(PageSize);

    while(1)
    {
        int size = 0;
        ssize_t c_read = read(fd, buffer, PageSize);
        if(c_read == -1) handle_error("Eroare la citirea din fisier");
        if(c_read == 0) break;
        for(int i=0;i<c_read;i++)
        {
            if(buffer[i] == '\n')
            {
                if(size > 0)
                {
                    if(sizeof(int) != write(pipe0to1[1], &size, sizeof(int)))
                        handle_error("Eorare la write in pipe0to1[1]");

                    if(size != write(pipe0to1[1], line, size))
                        handle_error("Eorare la write in pipe0to1[1]");
                }
                size = 0;
            }
            else line[size++] = buffer[i];
        }
        if(size > 0) // in caz ca ultima linie nu are new_line
        {
            if(sizeof(int) != write(pipe0to1[1], &size, sizeof(int)))
                handle_error("Eorare la write in pipe0to1[1]");

            if(size != write(pipe0to1[1], line, size))
                handle_error("Eorare la write in pipe0to1[1]");
        }

    }
    free(buffer);


    Close(pipe0to1[1]);

    printf("Sume de control: ");
    while(1)
    {
        int sum;
        ssize_t c_read = read(pipe2to0[0], &sum, sizeof(int));
        if(c_read == -1) handle_error("Eroare la citirea din pipe2to0[0]");
        if(c_read == 0) break;
        printf("%d ", sum);
    }

    Close(pipe2to0[0]);

    /// wait?
    return 0;
}