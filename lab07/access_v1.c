/*
* Program: access_v1.c
*
* Funcționalitate: versiunea 1 a programului demonstrativ de acces concurent/exclusiv a două procese la un fișier dat.
*/
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
//#include <string.h>
//#include <errno.h>

int main(int argc, char* argv[])
{
    int  fd, codRead;
    unsigned char ch;

    /* Verificarea existenței argumentului în linia de comandă. */
    if(argv[1] == NULL)
    {
        fprintf(stderr,"Programul %s trebuie apelat cu cel putin un parametru.\n", argv[0]);  exit(1);
    }

    /* Deschiderea fișierului de date. */
    if(-1 == (fd=open("fis.dat", O_RDWR)) )
    {   /* Tratează cazul de eroare la deschidere. */
        perror("Eroare la apelul open");
        // Echivalent: fprintf(stderr,"Nu pot deschide fisierul fis.dat: %s\n", strerror(errno));
        exit(2);
    }

    /* Bucla de parcurgere a fișierului, citind caracter cu caracter până la EOF. */
    while(0 != (codRead = read(fd,&ch,1)) )
    {
        if (-1 == codRead) { perror("Eroare la apelul read"); exit(3); } /* Tratează cazul de eroare la citire. */

        if(ch == '#')
        {   /* Am întâlnit prima poziție cu '#' în fișierul de date. */

            if(-1 == lseek(fd,-1L,1) ) { perror("Eroare la apelul lseek"); exit(4); } /* Tratează cazul de eroare la repoziționare. */

            sleep(5); // O pauză, pentru a forța anumite ordini la execuția concurentă a mai multor instanțe ale programului.

            if(-1 == write(fd,argv[1],1) ) { perror("Eroare la apelul write"); exit(5); } /* Tratează cazul de eroare la scriere. */

            printf("[ProcesID:%d] Terminat. S-a inlocuit primul '#' gasit in fisierul fis.dat !\n", getpid());
            return 0;
        }
    }// end bucla while
  
    close(fd);
    printf("[ProcesID:%d] Terminat. Nu exista '#' in fisierul fis.dat !\n", getpid());
    return 0;
}