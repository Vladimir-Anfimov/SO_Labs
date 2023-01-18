#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

#define FILE_NAME "numbers.bin"
#define MAX_COUNT 100
#define MAX_NUMBER 25
#define PAGE_SIZE 4096
#define VECTOR_SIZE_MAX 1024

#define handle_error(msg) \
    {perror(msg); exit(EXIT_FAILURE);}

void handle_sigusr(int number);

void generate_file();

pid_t Fork()
{
    pid_t pid;
    if(-1 == (pid = fork())) handle_error("Eroare la fork");
    return pid;
}

void Close(int descriptor)
{
    if(-1 == close(descriptor)) handle_error("Eroare la close descriptor");
}

int main()
{
    pid_t pidCh1, pidCh2;
    sigset_t masca_blocat;
    sigemptyset(&masca_blocat);
    sigaddset(&masca_blocat, SIGUSR1);

    if(-1 == sigprocmask(SIG_BLOCK, &masca_blocat, NULL))
        handle_error("Eroare la sigprocmask");

    if(SIG_ERR == signal(SIGUSR1, handle_sigusr))
        handle_error("Eroare la signal");

    int fd;
    sigset_t masca_deblocat;
    sigfillset(&masca_deblocat);
    sigdelset(&masca_deblocat, SIGUSR1);

    if(0 == (pidCh1 = Fork()))
    { // copil 1
        sigsuspend(&masca_deblocat);
        
        if(-1 == (fd = open(FILE_NAME, O_RDONLY))) handle_error("Eroare la open FILE_NAME in O_RDONLY");
        int sum = 0;
        while(1)
        {
            int buffer[PAGE_SIZE/sizeof(int)];
            ssize_t c_read = read(fd, &buffer, PAGE_SIZE/sizeof(int));
            if(c_read == -1) handle_error("Eroare la citire");
            if(c_read == 0) break;
            for(int i=0;i<c_read;i++)
            {
                sum += buffer[i];
            }
        }

        printf("Suma toatala este: %d\n", sum);
        Close(fd);
        exit(EXIT_SUCCESS);
    }

    if(0 == (pidCh2 = Fork()))
    { // copil 2
        sigsuspend(&masca_deblocat);
        
        if(-1 == (fd = open(FILE_NAME, O_RDONLY))) handle_error("Eroare la open FILE_NAME in O_RDONLY");
        int numbers[VECTOR_SIZE_MAX], size = 0;
        while(1)
        {
            int buffer[1024];
            ssize_t c_read = read(fd, &buffer, 1024);
            if(c_read == -1) handle_error("Eroare la citire");
            if(c_read == 0) break;
            for(int i=0;i<c_read;i++)
                numbers[size++] = buffer[i];
        }

        for(int i=0;i<size-1;i++)
        {
            for(int j=i+1;j<size;j++)
            {
                if(numbers[i] > numbers[j])
                {
                    int aux = numbers[i];
                    numbers[i] = numbers[j];
                    numbers[j] = aux;
                }
            }
        }

        sigsuspend(&masca_deblocat);

        printf("Numerele sortate sunt: ");
        for(int i=0;i<size;i++)
        {
            printf("%d ", numbers[i]);
        }
        printf("\n");
        Close(fd);
        exit(EXIT_SUCCESS);
    }

    generate_file();
    kill(pidCh1, SIGUSR1);
    kill(pidCh2, SIGUSR1);

    wait(NULL);
    kill(pidCh2, SIGUSR1);
    wait(NULL);

    return 0;

}

void generate_file()
{
    int fd;
    if(-1 == (fd = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0600)))
        handle_error("Eroare la open FILE_NAME in O_WRONLY | O_CREAT | O_TRUNC");

    srand(getpid());
    int count = 1 + random() % MAX_COUNT;

    for(int i=0; i<count;i++)
    {
        int number = 1 + random() % MAX_NUMBER;
        if(sizeof(int) != write(fd, &number, sizeof(int)))
            handle_error("Eroare la write in FILE_NAME");
    }
    close(fd);
}

void handle_sigusr(int number)
{
    // fprintf(stderr, "Sunt procesul %d si am primit semnalul %d\n", getpid(), number);
}