#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define handle_error(msg) \
    {perror(msg); exit(EXIT_FAILURE);}

void print()
{
    printf("%d-%d\n", getpid(), getppid());
}

int main(int argc, char **argv)
{
    int n, k;
    if(argc < 3)
        handle_error("Format apel: ./perftree nivele k-procese");
    if(1 != sscanf(argv[1], "%d", &n) || 1 != sscanf(argv[2], "%d", &k))
        handle_error("Input apel invalid");
    
    pid_t pid;
    if(-1 == (pid = fork()))
        handle_error("Eroare la creare primului nivel");
    
    if(pid == 0)
    {
        n--;
        print();
        int j = 0;
        while(j<k && n > 0)
        {
            pid_t newpid = fork();
            if(newpid == -1)
                handle_error("Eroare la fork");
            if(newpid == 0)
            {
                print();
                j = 0;
                n--;
            }
            else j++;
        }
        if(n == 0) exit(1);

        for(int j=0;j<k;j++)
            wait(NULL);
        exit(1);
    }  
    else wait(NULL);  
}