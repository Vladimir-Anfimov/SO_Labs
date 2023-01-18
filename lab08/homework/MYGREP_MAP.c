#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

#define handle_error(msg) \
    { perror(msg); exit(EXIT_FAILURE); }


void splitLines(char *map_addr, char *pattern, const char *file_name, int flag_c, int flag_v, int flag_n, int *match_number);

int main(int argc, char **argv)
{
    struct stat sb;
    int flag_c = 0, flag_v = 0, flag_n = 0;
    char *pattern = NULL;

    if(argc < 3)
    {
        fprintf(stderr, "Utilizare %s [OPTIUNI...] PATTERN [FISIER...] \n", argv[0]);
        return 1;
    }

    for(int i=1; i < argc - 1; i++)
    {
        if(strcmp(argv[i], "-c") == 0) flag_c = 1;
            else if(strcmp(argv[i], "-v") == 0) flag_v = 1;
                else if(strcmp(argv[i], "-n") == 0) flag_n = 1;
                    else if(pattern == NULL) {
                        unsigned length = strlen(argv[i]) + 1;
                        pattern = malloc(length);
                        memcpy(pattern, argv[i], length);                  
                    }
    }
    if(pattern == NULL)
        handle_error("Ati uitat sa specificati un pattern!\n");
    
    int dir_count = argc - 2 - flag_c - flag_n - flag_v;
    if(dir_count == 0)
        handle_error("Ati uitat sa specificati numele fisierului/fisierelor de prelucrat!\n");


    int match_number = 0;
    for(int i = argc - dir_count; i < argc; i++)
    {
        int descriptor;
        char *file_name = argv[i];

        if(-1 == (descriptor = open(argv[i], O_RDONLY)))
            handle_error("Eroare la deschiderea fisierului. ");
        
        if(-1 == fstat(descriptor, &sb))
            handle_error("Eroare la apelul fstat.");

        char *map_addr = mmap(NULL,
                              sb.st_size,
                              PROT_READ | PROT_WRITE,
                              MAP_PRIVATE,
                              descriptor,
                              0
        );
        if(map_addr == MAP_FAILED)
            handle_error("Eroare la mapare.");
        if(-1 == close(descriptor))
            handle_error("Eroare la inchiderea fisierului.");
        
        splitLines(map_addr, pattern, dir_count > 1 ? file_name : "", flag_c, flag_v, flag_n, &match_number);

        if( -1 == munmap(map_addr, sb.st_size))
            handle_error("Eroare la munmap.");
    }

    if(flag_c == 1)
    {
        printf("%d\n", match_number);
    }

    return 0;
}

void splitLines(char *map_addr, char *pattern, const char *file_name, int flag_c, int flag_v, int flag_n, int *match_number)
{
    char *line = strtok(map_addr, "\n");
    int line_number = 1;

    while(line != NULL)
    {
        char *find_result = strstr(line, pattern);
        if((find_result != NULL && flag_v == 0) || (find_result == NULL && flag_v == 1))
        {
            if(flag_c == 0) {
                if(flag_n == 1)
                {
                    printf("%d)", line_number);
                }
                if(strcmp(file_name, "") != 0)
                {
                    printf("%s: ", file_name);
                }
                printf("%s\n", line);
            }
            *match_number += 1;
        }
    
        line_number++;
        line = strtok(NULL, "\n");
    }
}

