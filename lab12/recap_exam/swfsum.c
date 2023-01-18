#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>

#define MAX_WORKERS 50
#define FIFO_SUM "fifo_sum"

#define handle_error(msg) \
    {perror(msg); exit(EXIT_FAILURE);}

void func_worker(char *fi);

void func_supervisor(int fo[MAX_WORKERS], int N);


int main(int argc, char **argv)
{
    int N, fdWorker[MAX_WORKERS];
    
    if(argc < 2)
    {
        fprintf(stderr, "Apelare: %s N\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    N = atoi(argv[1]);

    if(-1 == mkfifo(FIFO_SUM, 0600) && errno != EEXIST) handle_error("Eroare la mkfifo FIFO_SUM");

   for(int i=0;i<N;i++)
   {
        char fifo_name[PATH_MAX];
        sprintf(fifo_name, "fifo%iworker", i);
        if(-1 == mkfifo(fifo_name, 0600) && errno != EEXIST)
            handle_error("Eroare la mkfifo FIFO WORKER");

        switch (fork())
        {
        case -1:
            handle_error("Eroare la fork");
        case  0:    
            func_worker(fifo_name);
        default:
            if(-1 == (fdWorker[i] = open(fifo_name, O_WRONLY))) handle_error("Eorare la open fdWorker in O_WRONLY");
            break;
        }
   }

   func_supervisor(fdWorker, N);

   return 0;
}

void func_worker(char *fi)
{
    int fdIn, fdOut;
    if(-1 == (fdIn = open(fi, O_RDONLY))) handle_error("Eorare la open fi in O_RDONLY");

    // citirea pana la EOF
    int partial_sum = 0;
    while(1)
    {
        int number;
        ssize_t c_read = read(fdIn, &number, sizeof(int));
        if(c_read == 0) break;
        if(c_read != sizeof(int)) handle_error("Eroare la read din fdIn");
        partial_sum += number;
    }
    if(-1 == close(fdIn)) handle_error("Eroare la close fdIn");

    if(-1 == (fdOut = open(FIFO_SUM, O_WRONLY))) handle_error("Eroare la open fdOut in O_WRONLY");
    
    if(sizeof(int) != write(fdOut, &partial_sum, sizeof(int))) handle_error("Eroare la write in fdOut");

    if(-1 == close(fdOut)) handle_error("Eroare la close fdOut");

    exit(EXIT_SUCCESS);
}

void func_supervisor(int fo[MAX_WORKERS], int N)
{
    int number, i = 0, fdIn, sum = 0;
    printf("Scrie numerele: \n");
    while(1)
    {
        scanf("%d", &number);
        if(number == 0) break;
        if(sizeof(int) != write(fo[i], &number, sizeof(int))) handle_error("Eroare la write in fo");
        i = (i+1) % N;
    }

    for(int i=0;i<N;i++)
        if(-1 == close(fo[i])) handle_error("Eroare la close fo");

    if(-1 == (fdIn = open(FIFO_SUM, O_RDONLY))) handle_error("Eroare la open FIFO SUM in O_RDONLY");
    
    for(int i=0;i<N;i++)
        wait(NULL);
    
    while(1)
    {
        int c_read = read(fdIn, &number, sizeof(int));
        if(c_read == 0) break;
        if(c_read != sizeof(int)) handle_error("Eroare la read din FIFO_SUM");
        sum += number;
    }

    if(-1 == close(fdIn)) handle_error("Eroare la close FIFO SUM in supervisor");

    printf("Suma totala este: %d\n", sum);
    
}