#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#define handle_error(msg) \
    {perror(msg); exit(EXIT_FAILURE);}

pid_t Fork()
{
    pid_t pid = fork();
    if(pid == -1) handle_error("Eroare la fork");
    return pid;
}

void Pipe(int p[])
{
    if(-1 == pipe(p))
        handle_error("Eroare la pipe");
}

void Close(int descriptor)
{
    if(-1 == close(descriptor))
        handle_error("Eroare la close");
}

int main()
{
    pid_t pidCh1, pidCh2, pidCh3;
    int pipe1to0[2], pipe2to1[2], pipe3to2[2];

    Pipe(pipe1to0);

    if(0 == (pidCh1 = Fork()))
    {
        // copil 1
        Close(pipe1to0[0]);
        Pipe(pipe2to1);
        
        if(0 == (pidCh2 = Fork()))
        {
            // copil 2
            Close(pipe2to1[0]);
            Pipe(pipe3to2);

            if(0 == (pidCh3 = Fork()))
            {
                // copil 3
                Close(pipe3to2[0]);
                dup2(pipe3to2[1], 1);
                execlp("grep", "grep", "/bin/bash", "/etc/passwd", NULL);
                handle_error("Eroare la exclp in copil 3");
            }
            else {
                // copil 2
                Close(pipe3to2[1]);
                dup2(pipe3to2[0], 0);
                dup2(pipe2to1[1], 1);
                execlp("cut", "cut", "-d:", "-f1,3-5", NULL);
                handle_error("Eroare la exclp in copil 2");
            }
        }
        else {
            // copil 1
            Close(pipe2to1[1]);
            dup2(pipe2to1[0], 0);
            dup2(pipe1to0[1], 1);
            execlp("sort", "sort", "-t:", "-k 2,2", "-n", NULL);
            handle_error("Eroare la exclp in copil 1");
        }
    }
    else {
        // parinte
        Close(pipe1to0[1]);
        dup2(pipe1to0[0], 0);
        execlp("cut", "cut", "-d,", "-f1", NULL);
        handle_error("Eroare la exclp in copil 2");
    }
    return 0;
}