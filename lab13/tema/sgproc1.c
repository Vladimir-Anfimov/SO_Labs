#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <wait.h>
#include <errno.h>

#define handle_error(msg) \
    {perror(msg); exit(1);} 

pid_t pid = 0;

void handler_SIGINT(int nr_sem)
{
    kill(pid, SIGUSR2);
    wait(NULL);
    exit(0);
}

void handler_SIGUSR1(int nr_sem)
{
    printf("Am primit USR1\n");
    fflush(stdout);
}

void handler_SIGUSR2(int nr_sem)
{
    printf("Fiul: sfarsit executie\n");
    fflush(stdout);
    exit(0);
}


int main()
{
    if(SIG_ERR == signal(SIGINT, handler_SIGINT))
            handle_error("Eroare la signal: handler_SIGINT");

    if(-1 == (pid = fork()))
        handle_error("Eroare la fork.");
    
    if(pid == 0) //copil
    {  
        if(SIG_ERR == signal(SIGUSR1, handler_SIGUSR1))
            handle_error("Eroare la signal: handler_SIGUSR1");
        while(1){};
    }
    else { //parinte
        if(SIG_ERR == signal(SIGUSR2, handler_SIGUSR2))
            handle_error("Eroare la signal: handler_SIGUSR2");
        int count = 0;
        while(1)
        {
            count++;
            if(count % 10 == 0)
            {
                kill(pid, SIGUSR1);
            }
            sleep(1);
        }
    }

    return 0;
}