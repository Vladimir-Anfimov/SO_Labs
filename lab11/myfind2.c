#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <pwd.h>

int main(int argc, char **argv)
{
    int cod_term;
    switch (fork())
    {
    case -1:
        perror("Eroare la fork");
        exit(2);
    case 0:
    {
        struct passwd *pw = getpwuid(getuid());

        char *cmds[20]={"exec_find", pw->pw_dir, "-name", "*.c", "-printf", "\"%p : %k KB\n\"", NULL};
        execv("/usr/bin/find", cmds);
        perror("Eroare la exec.");
        exit(10);
    }
    default:
        wait(&cod_term);
        if(WIFEXITED(cod_term))
        {
            switch (WEXITSTATUS(cod_term))
            {
                case 10:  printf("Comanda find nu a putut fi executata...\n");  break;
                case  0:  printf("Comanda find a fost executata ...\n");  break;
                default:  printf("Comanda find a fost executata, dar a esuat, terminandu-se cu codul de terminare: %d.\n", WEXITSTATUS(cod_term) );
            }
        }
        else {
            printf("Comanda find a fost terminata fortat de catre semnalul: %d.\n", WTERMSIG(cod_term) );
        }
    }

    return 0;
}