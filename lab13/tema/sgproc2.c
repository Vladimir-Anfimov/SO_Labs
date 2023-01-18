#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <wait.h>

#define handle_error(msg) \
    {perror(msg); exit(0); }

#define FILE_NAME "data.txt"
#define MAX_NR_LINES 25
#define MAX_CHARS_LINE 50
#define PAGE_SIZE 4096

void generate_random_text();

void handle_signal(int sign);

int main()
{
    pid_t pidOne, pidTwo;
    sigset_t masca_blocat;

    sigemptyset(&masca_blocat);
    sigaddset(&masca_blocat,SIGUSR1);
    sigaddset(&masca_blocat,SIGUSR2);

    if(-1 == sigprocmask(SIG_BLOCK, &masca_blocat, NULL) )
        handle_error("Eroare la sigprocmask");

    if(SIG_ERR == signal(SIGUSR1, handle_signal) )
        handle_error("Eroare la signal");
    
    if(SIG_ERR == signal(SIGUSR2, handle_signal) )
        handle_error("Eroare la signal");

    if(-1 == (pidOne = fork()))
        handle_error("Eroare la fork");

    if(pidOne == 0)
    {
        sigset_t masca_deblocat;
        sigfillset(&masca_deblocat);
        sigdelset(&masca_deblocat, SIGUSR2);

        sigsuspend(&masca_deblocat); //astepam semnalul SIGURS2
        
        ssize_t c_read;

        int fd;
        if(-1 == (fd = open(FILE_NAME, O_RDONLY)))
            handle_error("Eroare la open write read file");

        int lines = 0;
        char buffer[PAGE_SIZE];
        do {
            if(-1 == (c_read = read(fd, buffer, PAGE_SIZE)))
                handle_error("Eroare la citirea din fisier");
            if(c_read != 0)
            {
                for(int i=0;i<c_read; i++)
                    if(buffer[i] == '\n')
                        lines++;
            }
        } while(c_read != 0);

        printf("Numarul de linii: %d\n", lines);
        fflush(stdout);
        close(fd);
        exit(1);
    }

    if(-1 == (pidTwo = fork()))
        handle_error("Eroare la fork");

    if(pidTwo == 0)
    {
        sigset_t masca_deblocat;
        sigfillset(&masca_deblocat);
        sigdelset(&masca_deblocat, SIGUSR1);

        sigsuspend(&masca_deblocat); //astepam semnalul SIGURS1
        
        int fd;
        if(-1 == (fd = open(FILE_NAME, O_RDONLY)))
            handle_error("Eroare la open write read and creat file");

        char lines[MAX_NR_LINES][MAX_CHARS_LINE];
        int lines_total = 0;
        int c_read;
        int index=0;
        char buffer[PAGE_SIZE];
        do {
            if(-1 == (c_read = read(fd, buffer, PAGE_SIZE)))
                handle_error("Eroare la citirea din fisier");
            if(c_read != 0)
            {
                for(int i=0;i<c_read; i++)
                    if(buffer[i] == '\n')
                    {
                        lines[lines_total++][index] = '\0';
                        index = 0;
                    }
                    else lines[lines_total][index++] = buffer[i];
            }
        } while(c_read != 0);

        for(int i=0;i<lines_total-1;i++)
        {
            for(int j=i+1;j<lines_total;j++)
            {
                if(strcmp(lines[i], lines[j]) > 0)
                {
                    char aux[MAX_CHARS_LINE];
                    strcpy(aux, lines[i]);
                    strcpy(lines[i], lines[j]);
                    strcpy(lines[j], aux);
                }
            }
        }

        sigsuspend(&masca_deblocat); //astepam semnalul SIGURS2
                                     // din partea celuilalt copil
        
        for(int i=0;i<lines_total;i++)
        {
            printf("%s\n", lines[i]);
            fflush(stdout);
        }

        close(fd);
        exit(1);
    }
    // parinte
    generate_random_text();

    kill(pidOne, SIGUSR2);
    kill(pidTwo, SIGUSR1);
   
    wait(NULL);
    kill(pidTwo, SIGUSR1);

    wait(NULL);

    return 0;
}

void generate_random_text()
{
    srand(getpid());
    
    int fd;
    if(-1 == (fd = open(FILE_NAME, O_RDWR | O_CREAT | O_TRUNC)))
        handle_error("Eroare la open write read and creat file");

    int total_lines = random() % MAX_NR_LINES;
    for(int line=1;line<= total_lines; line++)
    {
        char word[MAX_CHARS_LINE];
        int size = random() % (MAX_CHARS_LINE - 1);

        for(int index=0; index<size; index++)
        {
            word[index] = 65 + random() % 26;
        }
        word[size] = '\n';
        if(sizeof(char)*(size+1) != write(fd, word, sizeof(char)*(size+1)))
                handle_error("Eroare la write in file");
    }

    close(fd);
}

void handle_signal(int sign) 
{
    //fprintf(stderr,"Debug: procesul cu PID-ul %d a primit semnalul de notificare.\n", getpid());
}