//['Supervisor-workers' pattern #1N: A coordinated distributed sum #1N (v1, using regular files for IPC)]
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <string.h>

#define MAX_WORKERS 50

#define handle_error(msg) \
    {perror(msg); exit(EXIT_FAILURE);}

int main(int argc, char **argv)
{
    int fdIn, workers, PageSize;
    char file_in[PATH_MAX];
    int pipesWorkerToSuper[MAX_WORKERS][2], pipesSuperToWorker[MAX_WORKERS][2];
    struct stat sb;

    if(argc < 3)
        fprintf(stderr, "Format apel: ./swpsum fisier.bin instante\n"), exit(1);
    
    workers = atoi(argv[2]);

    if(workers > MAX_WORKERS || workers <= 0) 
        fprintf(stderr, "Numarul maxim de workeri este %d si trebuie sa fie mai mare ca 0\n", MAX_WORKERS), exit(1);

    strcpy(file_in, argv[1]);

    if(-1 == (fdIn = open(file_in, O_RDONLY)))
        handle_error("Eroare la deschiderea fisierului de intrare");

    if(-1 == fstat(fdIn, &sb))
        handle_error("Eroare la fstat");

    if(workers > sb.st_size / sizeof(int))
        workers = sb.st_size / sizeof(int);

    if(-1 == (PageSize = sysconf(_SC_PAGE_SIZE)))
        handle_error("Eroare la sysconf");

    for(int i=0;i<workers;i++)
    {
        if(-1 == pipe(pipesSuperToWorker[i]) || -1 == pipe(pipesWorkerToSuper[i]))
            handle_error("Eroare la pipe");
        
        pid_t pid = fork();
        if(pid == -1) handle_error("Eroare la fork");
        if(pid == 0)
        {
            if(-1 == close(pipesSuperToWorker[i][1])) 
                handle_error("Eroare la close pipesSuperToWorker[1]");
            if(-1 == close(pipesWorkerToSuper[i][0]))
                handle_error("Eroare la close pipesWorkerToSuper[0]");
            int sum = 0;
            while(1)
            {
                int number;
                int c_read = read(pipesSuperToWorker[i][0], &number, sizeof(int));
                if(c_read == 0) break;
                if(c_read != sizeof(int)) handle_error("Eroare la citirea din pipesSuperToWorker[0]"); 
                sum += number;
            }   
            if(-1 == close(pipesSuperToWorker[i][0]))
                handle_error("Eroare la close pipesSuperToWorker[0]");
            if(sizeof(int) != write(pipesWorkerToSuper[i][1], &sum, sizeof(int)))
                handle_error("Eroare la scrierea in pipesWorkerToSuper[1]");
            if(-1 == close(pipesWorkerToSuper[i][1]))
                handle_error("Eroare la close pipesWorkerToSuper[1]");
            exit(1);
        }
        else {
            if(-1 == close(pipesSuperToWorker[i][0])) 
                handle_error("Eroare la close pipesSuperToWorker[0]");
            if(-1 == close(pipesWorkerToSuper[i][1]))
                handle_error("Eroare la close pipesWorkerToSuper[1]");
        }
    }


    int turn = 0;
    while(1)
    {
        int buffer[PageSize / sizeof(int)];
        int c_read = read(fdIn, buffer, PageSize / sizeof(int));
        if(c_read == -1) handle_error("Eroare la citire din fisierul de intrare");
        if(c_read == 0) break;
        for(int i=0;i<c_read;i++, turn = (turn+1) % workers)
        {
            if(sizeof(int) != write(pipesSuperToWorker[turn][1], &buffer[i], sizeof(int)))
                handle_error("Eroare la scrierea in pipesSuperToWorker[1]");
        }
    }

    if(-1 == close(fdIn))
        handle_error("Eroare la inchiderea fisierului de intrare");

    for(int i=0;i<workers;i++)
    {
        if(-1 == close(pipesSuperToWorker[i][1]))
            handle_error("Eroare la close pipesSuperToWorker[1]");
    }

    int total_sum = 0;
    for(int i=0;i<workers;i++)
    {
        int sum;
        if(sizeof(int) != read(pipesWorkerToSuper[i][0], &sum, sizeof(int)))
            handle_error("Eroare la citirea din pipesWorkerToSuper[0]"); 
        if(-1 == close(pipesWorkerToSuper[i][0]))
            handle_error("Eroare la close pipesWorkerToSuper[0]");
        total_sum += sum;
    }

    for(int i=0;i<workers;i++)
        wait(NULL);
    printf("Suma totala este: %d\n", total_sum);

    return 0;
}