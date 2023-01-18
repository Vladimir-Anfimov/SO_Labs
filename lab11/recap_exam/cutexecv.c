#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define handle_error(msg) \
    {perror(msg);exit(EXIT_FAILURE);}

int main()
{
    pid_t pid = fork();
    switch (pid)
    {
        case -1:
            handle_error("Eroare la fork");
        case 0:
        {
            char *argv[] ={ "cut", "-f1,3", "-d:", "--output-delimiter= - ", "/etc/group", NULL };
            execv("/usr/bin/cut", argv);
            perror("Eroare la execv");
            exit(10);
        }
        default:
        {
            int status;
            wait(&status);
            if((status & 0xFF) == 0)
            {
                int cod = (status >> 8) & 0xFF;
                if(cod == 10)
                {
                    handle_error("Comanda cut nu a putut fi executata (apelul execv a esuat) COD_TERM 10");
                }
                else if(cod == 0)
                {
                    printf("Comanda cut a fost executa cu succes.");
                }
                else {
                    fprintf(stderr, "Comanda cut a fost executata, dar a esuat cu codul de eroare %d", cod);
                }
            }
            else {
                fprintf(stderr, "Comanda cut a fost terminata format de catre semnalul %d", status & 0xFF);
            }
            printf("\n");
        }
    }

    return 0;
}