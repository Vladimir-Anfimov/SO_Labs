#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_DIVIZORI 100

#define handle_error(msg) \
    {perror(msg); exit(EXIT_FAILURE);}

int isPrime(int number);

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

int main()
{
    int pipe0to1[2], pipe1to0[2], pipe0to2[2], pipe2to0[2];
    pid_t pidCh1, pidCh2;

    Pipe(pipe0to1);
    Pipe(pipe1to0);

    if(0 == (pidCh1 = Fork()))
    {   // copil 1
        Close(pipe0to1[1]);
        Close(pipe1to0[0]);

        int number;
        while(1)
        {
            int c_read = read(pipe0to1[0], &number, sizeof(int));
            if(c_read == -1) handle_error("Eroare la citirea din pipe");
            if(c_read == 0) break;
            int status = isPrime(number);
            if(sizeof(int) != write(pipe1to0[1], &status, sizeof(int))) handle_error("Eroare la scrierea in pipe");
        }
        
        Close(pipe0to1[0]);
        Close(pipe1to0[1]);

        exit(EXIT_SUCCESS);
    }

    Close(pipe0to1[0]);

    Pipe(pipe0to2);
    Pipe(pipe2to0);

    if(0 == (pidCh2 = Fork()))
    {   // copil 2
        Close(pipe0to2[1]);
        Close(pipe2to0[0]);
        Close(pipe0to1[1]);

        int number;
        while(1)
        {
            int c_read = read(pipe0to2[0], &number, sizeof(int));
            if(c_read == -1) handle_error("Eroare la citirea din pipe");
            if(c_read == 0) break;
            int size = 0, divizori[MAX_DIVIZORI];
            for(int d=1; d<=number;d++)
            {
                if(number % d == 0)
                    divizori[size++] = d;
            }
            if(sizeof(int) != write(pipe2to0[1], &size, sizeof(int))) handle_error("Eroare la scrierea in pipe");
            for(int i = 0; i < size; i++)
            {
                if(sizeof(int) != write(pipe2to0[1], &divizori[i], sizeof(int))) handle_error("Eroare la scrierea in pipe");
            }
        }

        Close(pipe0to2[0]);
        Close(pipe2to0[1]);

        exit(EXIT_SUCCESS);
    }

    Close(pipe0to2[0]);
    Close(pipe2to0[1]);

    // parinte
    int number;
    while(1)
    {
        scanf("%d", &number);
        if(number == 0) break;
        if(sizeof(int) != write(pipe0to1[1], &number, sizeof(int))) handle_error("Eroare la scrierea in pipe");

        int prim;
        if(sizeof(int) != read(pipe1to0[0], &prim, sizeof(int))) handle_error("Eroare la citirea din pipe");
        
        if(prim == 1) printf("Numarul %d este prim.\n", number);
        else {
            if(sizeof(int) != write(pipe0to2[1], &number, sizeof(int))) handle_error("Eroare la scrierea in pipe");
            int size;
            if(sizeof(int) != read(pipe2to0[0], &size, sizeof(int))) handle_error("Eroare la citirea din pipe");
            printf("Numarul %d are divizorii: ", number);
            for(int i=0, divizor;i<size;i++)
            {
                if(sizeof(int) != read(pipe2to0[0], &divizor, sizeof(int))) handle_error("Eroare la citirea din pipe");
                printf("%d, ", divizor);
            }
            printf("\n");
        } 
    }

    Close(pipe0to1[1]);
    Close(pipe1to0[0]);
    Close(pipe0to2[1]);
    Close(pipe2to0[0]);

    return 0;
}


int isPrime(int number)
{
    if(number == 1) return 0;
    for(int k = 2; k*k<=number; k++)
    {
        if(number % k == 0)
            return 0;
    }
    return 1;
}