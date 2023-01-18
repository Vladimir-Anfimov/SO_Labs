#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define BLOCK_SIZE 4096

struct Pair {
    int key, value;
};

int modificare_pereche(int fdPer, struct Pair modf_pair, struct flock lacat);

void afisare_perechi(int fd);

int main(int argc, char **argv)
{
    int fdPer, fdComp;

    if(argc != 3)
    {
        fprintf(stderr, "Fisierul trebuie apelat cu 2 exact parametrii in urmatoarele 2 feluri: \n");
        fprintf(stderr, "1) [perechi.bin] [comenzi.bin]\n");
        fprintf(stderr, "2) [perechi.bin] -o\n");
        return 1;
    }

    if(-1 == (fdPer = open(argv[1], O_RDWR)))
    {
        fprintf(stderr, "Eroare la deschiderea fisierului %s\n", argv[1]);
        return 2;
    }

    if(strcmp(argv[2],"-o") == 0)
    {
        afisare_perechi(fdPer);
        return 3;
    }

    if(-1 == (fdComp = open(argv[2], O_RDONLY)))
    {
        fprintf(stderr, "Eroare la deschiderea fisierului %s\n", argv[2]);        
        return 4;
    }

    struct flock lacat;
    lacat.l_type = F_WRLCK;
    lacat.l_whence = SEEK_CUR;
    lacat.l_start = (-1)*sizeof(struct Pair);
    lacat.l_len = sizeof(struct Pair);

    struct Pair pair;
    
    while(1)
    {
        int bytes_read;
        bytes_read = read(fdComp, &pair, sizeof(struct Pair));
        if(bytes_read == -1)
        {
            perror("Eroare la citirea unei perechi din comenzi.\n");
            return 0;
        }
        if(bytes_read == 0)
        {
            close(fdPer);
            close(fdComp);
            return 0;
        }
        /// de analizat return-ul
        if(-1 == modificare_pereche(fdPer, pair, lacat))
        {
            break;
        }
    }    

    close(fdPer);
    close(fdComp);
    
}

int modificare_pereche(int fdPer, struct Pair modf_pair, struct flock lacat)
{
    if(-1 == lseek(fdPer, 0, SEEK_SET))
    {
        perror("Eroare la apelul lseek. ");
        return -1;
    }
    
    while(1)
    {
        int bytes_read;
        struct Pair pair;
        bytes_read = read(fdPer, &pair, sizeof(struct Pair));
        if(bytes_read == -1)
        {
            perror("Eroare la citirea unei perechi din baza principala de perechi.\n");
            return -1;
        }
        if(bytes_read == 0)
        {
            return 1;
        }
        if(pair.key == modf_pair.key)
        {
            printf("[ProcessID:%d] Incep apelul de punere a blocajului in mod blocant pe perechea (%d, %d).\n", getpid(), pair.key, pair.value);
            if(-1 == fcntl(fdPer, F_SETLKW, &lacat))
            {
                if(errno == EINTR) 
                    fprintf(stderr,"[ProcesID:%d] Eroare, apelul fcntl a fost intrerupt de un semnal...\n", getpid());
                else
                    fprintf(stderr,"[ProcesID:%d] Eroare la blocaj...\n", getpid());
                perror("Motivul erorii este:");
                return -1;
            }
            else printf("[ProcessID:%d] Blocaj reusit!\n", getpid());

            sleep(1);

            if(-1 == lseek(fdPer, -sizeof(struct Pair), SEEK_CUR))
            {
                perror("Eroare la apelul lseek. ");
                return -1;
            }
            if(-1 == read(fdPer, &pair, sizeof(struct Pair)))
            {
                perror("Eroare la citirea unei perechi din baza principala de perechi.\n");
                return -1;
            }
            if(-1 == lseek(fdPer, -sizeof(struct Pair), SEEK_CUR))
            {
                perror("Eroare la apelul lseek. ");
                return -1;
            }

            modf_pair.value +=  pair.value;
            if(-1 == write(fdPer, &modf_pair, sizeof(struct Pair)) )
            {
                perror("Eroare la apelul write in baza principala de perechi.");
                return -1;
            }
            printf("[ProcesID:%d] Terminat. S-a inlocuit perechea (%d, %d) cu (%d, %d).\n", getpid(), 
                    pair.key, pair.value, modf_pair.key, modf_pair.value);
            
            
            
            lacat.l_type = F_UNLCK;
            if((-1 == fcntl(fdPer,F_SETLK, &lacat))  && ((errno == EACCES)||(errno == EAGAIN)))
            {
                perror("Eroare critica la deblocarea fisierului cu baza de perechi: "); return -1;
            }
             printf("[ProcesID:%d] Lacatul de pe perechea nou modificata (%d, %d) a fost scos.\n", getpid(), 
                    modf_pair.key, modf_pair.value);
            return 1;
            
        }
    }    
}

void afisare_perechi(int fd)
{   
    while(1)
    {
        int bytes_read;
        struct Pair pair;
        bytes_read = read(fd, &pair, sizeof(struct Pair));
        if(bytes_read == -1)
        {
            perror("Eroare la citirea unei perechi din baza principala de perechi.\n");
            return;
        }
        if(bytes_read == 0)
        {
            printf("\n");
            close(fd);
            return;
        }
        printf("(%d, %d)\n", pair.key, pair.value);
    }    
}
