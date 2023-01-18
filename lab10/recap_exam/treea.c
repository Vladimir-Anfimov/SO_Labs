#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#define handle_error(msg) \
    {perror(msg); exit(EXIT_FAILURE);}

void print()
{
    printf("%d-%d\n", getpid(), getppid());
}

int main()
{
    pid_t first_lvl = fork();

    srand(getpid());

    switch (first_lvl)
    {
        case -1:
            handle_error("Eroare la fork pe nivelul 0");
        
        case 0:
        {
            print();
            for(int i=0;i<2;i++)
            {
                pid_t second_lvl = fork();
                sleep(random() % 4);
                if(second_lvl == -1)
                    handle_error("Eroare la fork pe nivelul 1");
                if(second_lvl == 0)
                {
                    print();
                    for(int j=0;j<5;j++)
                    {
                        pid_t third_lvl = fork();
                        sleep(random() % 4);
                        if(third_lvl == -1)
                            handle_error("Eorare la fork pe nivelul 2");
                        if(third_lvl == 0)
                        {
                            print();
                            exit(1);   
                        }
                    }
                    for(int k=0;k<5;k++)
                        wait(NULL);
                    exit(1);
                }
            }
            for(int i=0;i<2;i++)
                wait(NULL);
            exit(1);
        }
        
        default:
            wait(NULL);
    }
}