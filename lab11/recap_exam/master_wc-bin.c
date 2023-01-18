#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define handle_error(msg) \
    {perror(msg); exit(EXIT_FAILURE);}

int main(int argc, char **argv)
{
    int fdIn, workers;
    struct stat sb;
    char file_in[PATH_MAX];
    
    if(argc < 3)
        fprintf(stderr, "Format apel: ./master_wc-bin fisier.bin instante\n"), exit(1);
    
    workers = atoi(argv[2]);
    int *fdOut = malloc(sizeof(int)*workers);

    strcpy(file_in, argv[1]);

    if(-1 == (fdIn = open(file_in, O_RDONLY)))
        handle_error("Eroare la deschiderea fisierului de intrare");

    if(-1 == fstat(fdIn, &sb))
        handle_error("Eroare la fstat");
    
    
    int work_quantity =  sb.st_size / workers;
    int offset = 0;
    
    for(int i=0;i<workers;i++)
    {
        char file_out[PATH_MAX];
        sprintf(file_out, "file_work%i.txt", i);
        if(-1 == (fdOut[i] = open(file_out, O_RDWR | O_CREAT | O_TRUNC, 0600)))
            handle_error("Eroare la crearea fisierului file_work");
        if(-1 == ftruncate(fdOut[i], sizeof(int)))
            handle_error("Eroare la ftruncate");
        
        pid_t pid = fork();
        switch (pid)
        {
        case -1:
            handle_error("Eroare la fork");
        case 0:
            if(i == workers - 1) work_quantity += sb.st_size % workers;
            char offset_str[20], length_str[20], descriptor_out_str[20];
            sprintf(offset_str, "%d", offset);
            sprintf(length_str, "%d", work_quantity);
            sprintf(descriptor_out_str, "%d", fdOut[i]);
            

            execl("./slave", "slave", file_in, descriptor_out_str, offset_str, length_str, NULL);
            perror("Eroare la execv");
            exit(EXIT_FAILURE);

        default:
            break;
        }
        offset += work_quantity;
    }
    for(int i=0;i<workers;i++)
        wait(NULL);

    int total_count = 0;

    for(int i=0;i<workers;i++)
    {
        int count;
        int c_read = read(fdOut[i], &count, sizeof(int));
        if(c_read != sizeof(int))
            handle_error("Eroare la citirea din fisierele out");
        total_count += count;
        close(fdOut[i]);
        char file_out[PATH_MAX];
        sprintf(file_out, "file_work%i.txt", i);
        if(-1 == remove(file_out))
            handle_error("Eroare la stergerea fisierelor");
    }

    printf("Au fost numarate %d caractere afla-numerice\n", total_count);

    free(fdOut);
    close(fdIn);

    return 0;
}