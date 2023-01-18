#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define handle_error(msg) \
    {fprintf(stderr, "Eroare: %s, errno=%d\n", msg, errno);  perror("Cauza erorii");  exit(1);}

int main()
{
	pid_t pid1, pid2, pid3;
	int pipeOneTwo[2], pipeTwoThree[2], pipeThreeFour[2];

	if(-1 == pipe(pipeThreeFour)) 
        handle_error("nu pot crea al treilea canal anonim");

    if(-1 == (pid1 = fork())) 
        handle_error("nu am putut crea primul fiu");

    if(pid1 == 0)
    {
        if(-1 == pipe(pipeTwoThree)) 
            handle_error("nu pot crea al doilea canal anonim");

        if(-1 == (pid2 = fork())) 
            handle_error("nu am putut crea al doilea fiu");
        
            if(pid2 == 0)
            {
                if(-1 == pipe(pipeOneTwo)) 
                    handle_error("nu pot crea primul canal anonim");

                if(-1 == (pid3 = fork())) 
                    handle_error("nu am putut crea al treilea fiu");
                
                    if(pid3 == 0)
                    {
                        if(-1 == dup2(pipeOneTwo[1], 1))
                            handle_error("redirectarea iesirii standard din al treilea fiu a esuat.");
                        
                        execlp("w", "w", "-h", NULL);
                        handle_error("execlp() nu a functionat in al treilea proces fiu");
                    }
                    else {
                        close(pipeOneTwo[1]);

                        if(-1 == dup2(pipeOneTwo[0], 0))
                            handle_error("redirectarea intrarii standard in al doilea fiu a esuat");
                        
                        if(-1 == dup2(pipeTwoThree[1], 1))
                            handle_error("redirectarea iesirii standard din al doilea fiu a esuat.");
                        execlp("tr", "tr", "-s", "\" \"", NULL);
                        handle_error("execlp() nu a functionat in al doilea proces fiu");
                    }
            }
            else {
                close(pipeTwoThree[1]);

                if(-1 == dup2(pipeTwoThree[0], 0))
                    handle_error("redirectarea intrarii standard primul fiu a esuat");
                
                if(-1 == dup2(pipeThreeFour[1], 1))
                    handle_error("redirectarea iesirii standard din primul fiu a esuat.");
                execlp("cut", "cut" , "-d", " ", "-f1,8", NULL); 
                handle_error("execlp() nu a functionat in primul proces fiu");
            }
    }
    else {
        close(pipeThreeFour[1]);

        if(-1 == dup2(pipeThreeFour[0], 0))
            handle_error("redirectarea intrarii standard in parinte a esuat");
                
        execlp("sort", "sort", "-t", " ", "-k 1", NULL);
        handle_error("execlp() nu a functionat in parinte");
    }

    return 0;
}