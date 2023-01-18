#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#define FILE_IN "date.txt"
#define FILE_OUT "statistica.txt"

#define handle_error(msg) \
    {perror(msg); exit(EXIT_FAILURE);}

pid_t c_fork()
{
    pid_t pid = fork();
    if(pid == -1) handle_error("Eroare la fork");
    return pid;
}

int c_open(const char* file_name, int flags)
{
    int fd;
    if(-1 == (fd = open(file_name, flags)))
        handle_error("Eroare la open");
    return fd;
}

void c_pipe(int _pipe[2])
{
    if(-1 == pipe(_pipe))
        handle_error("Eroare la crearea canalului");
}

void c_close(int fd, const char* fd_name)
{
    if(-1 == close(fd))
    {
        fprintf(stderr, "Eroare la inchiderea descriptorului %s\n", fd_name);
        handle_error("");
    }
}

int main()
{
    int pipe0to1[2], pipe1to2[2], pipe2to0[2], fdIn, PageSize;

    if(-1 == (PageSize = sysconf(_SC_PAGE_SIZE)))
        handle_error("Eroare la sysconf");
    
    fdIn = c_open(FILE_IN, O_RDONLY);
    
    c_pipe(pipe0to1);
    c_pipe(pipe2to0);

    pid_t pidCh1 = c_fork();

    if(pidCh1 == 0)
    {   
        c_close(pipe0to1[1], "pipe0to1[1]");
        c_close(pipe2to0[0], "pipe2to0[0]");

        c_pipe(pipe1to2);

        int pidCh2 = c_fork();
        if(pidCh2 == 0)
        {
            c_close(pipe1to2[1], "pipe1to2[1]");

            int freq[26] = {0};

            while(1)
            {
                char chr;
                int c_read = read(pipe1to2[0], &chr, sizeof(char));
                if(c_read == -1) handle_error("Eroare la citirea din canal pipe1to2[0]");
                if(c_read == 0) break;
                freq[chr - 'a']++;
            }

            int fdOut;
            if(-1 == (fdOut = open(FILE_OUT, O_WRONLY | O_CREAT | O_TRUNC, 0600)))
                handle_error("Eroare la open FILE_OUT");

            int distincte = 0;
            for(int i=0;i < 26;i++)
            {
                if(freq[i] != 0)
                {
                    distincte++;
                    char line[20];
                    sprintf(line, "%c %d\n", i+'a', freq[i]);
                    if(strlen(line) != write(fdOut, line, strlen(line)))
                        handle_error("Eroare la write in FILE_OUT");
                }
            }

            if(sizeof(int) != write(pipe2to0[1], &distincte, sizeof(int)))
                    handle_error("Eroare la write in pipe2to0[1]");

            c_close(pipe1to2[0], "pipe1to2[0]"); 
            c_close(pipe2to0[1], "pipe2to0[1]");

            c_close(fdOut, "fdOut");
            exit(2);
        }
        
        while(1)
        {
            char chr;
            int c_read = read(pipe0to1[0], &chr, sizeof(char));
            if(c_read == -1) handle_error("Eroare la citirea din canal pipe0to1[0]");
            if(c_read == 0) break;
            if(islower(chr) && sizeof(char) != write(pipe1to2[1], &chr, sizeof(char)))
                handle_error("Eroare la scrierea in canal pipe1to2[1]"); 
        }
        c_close(pipe1to2[1], "pipe1to2[1]");
        c_close(pipe0to1[0],"pipe0to1[0]");
        wait(NULL);
        exit(1);
    }

    c_close(pipe0to1[0], "pipe0to1[0]");
    c_close(pipe2to0[1], "pipe2to0[1]");

    char *buffer = malloc(PageSize);
    while(1)
    {
        int c_read = read(fdIn, buffer, PageSize);
        if(c_read == -1) handle_error("Eroare la citirea din FILE IN");
        if(c_read == 0) break;
        for(int i=0;i<c_read;i++)
        {
            if(sizeof(char) != write(pipe0to1[1], &buffer[i], sizeof(char)))
                handle_error("Eroare la scrierea in canalul pipe0to1[1]");
        }
    }
    free(buffer);
    c_close(pipe0to1[1], "pipe0to1[1]");

    int distincte;

    if(sizeof(int) != read(pipe2to0[0], &distincte, sizeof(int)))
        handle_error("Eroare la citirea din canalul pipe2to0[0]");
    c_close(pipe2to0[0], "pipe2to0[0]");
    wait(NULL);
    printf("Numarul de litere distincte este: %d\n", distincte);


    return 0;
}