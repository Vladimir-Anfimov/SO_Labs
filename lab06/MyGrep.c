#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define BLOCK_SIZE 4096

void splitLines(char *text, char *pattern, const char *file_name, int flag_c, int flag_v, int flag_n, int *match_number);
// Declaratia functiei urmeaza mai jos dupa main

struct FILE {
    int descriptor;
    char *name;
};

struct FILES_DIR {
    int count;
    struct FILE *file;
};

int main(int argc, char **argv)
{
    struct FILES_DIR dir;
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
    {
        fprintf(stderr,"Ati uitat sa specificati un pattern!\n");
        return 2;
    }
    
    dir.count = argc - 2 - flag_c - flag_n - flag_v;
    if(dir.count == 0)
    {
        fprintf(stderr,"Ati uitat sa specificati numele fisierului/fisierelor de prelucrat!\n");
        return 3;
    }

    dir.file = malloc(dir.count * sizeof(struct FILE));

    for(int i = argc - dir.count, j = 0; i < argc; i++, j++)
    {
        int size = strlen(argv[i]) + 1;
        dir.file[j].name = malloc(size);
        memcpy(dir.file[j].name, argv[i], size);
        if(-1 == (dir.file[j].descriptor = open(argv[i], O_RDONLY)))
        {
            perror("Eroare: ");
            return 4;
        }
        printf("Fisierul %d in care se va face cautarea este: %s\n", j + 1, dir.file[j].name);
    }
    printf("\n");


    int match_number = 0;
    for(int i = 0; i < dir.count; i++)
    {
        char buffer[BLOCK_SIZE];
        int bytes_read;

        while(1)
        {
            bytes_read = read(dir.file[i].descriptor, buffer, BLOCK_SIZE);
            if(bytes_read == 0) break;
            if(bytes_read == -1)
            {
                perror("Eroare: ");
                return 5;
            }
            splitLines(buffer, pattern, dir.count > 1 ? dir.file[i].name : "", flag_c, flag_v, flag_n, &match_number);
        }
    }

    if(flag_c == 1)
    {
        printf("%d\n", match_number);
    }

    return 0;
}

void splitLines(char *text, char *pattern, const char *file_name, int flag_c, int flag_v, int flag_n, int *match_number)
{
    char *line = strtok(text, "\n");
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

