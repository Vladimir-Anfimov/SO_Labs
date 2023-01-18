/*
   Iată o soluție incompletă, în sensul că s-a omis codul efectiv de punere și de scoatere a lacătului pentru acces exclusiv la fișierul de date partajat,
   precum și tratarea anumitor excepții (i.e., atunci când se încearcă extragerea unei cantități mai mari decât cea disponibilă în rezervorul stației PECO).
*/
// TODO: respectivele omisiuni vă rămân ca temă să le completați dvs. !!!

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <fcntl.h>

void initializare_fisier_date();
/*
*  Pentru început, avem nevoie de o rutină ajutătoare, care să creeze conținutul inițial al fișierului de date partajat.
*  Se va crea fișierul și va conține un număr real, stocat în format binar, reprezentând cantitatea inițială de combustibil din rezervor.
*  Vom apela programul cu opțiunea -i pentru a invoca această rutină de inițializare.
*/

void afisare_fisier_date();
/*
*  Similar, la finalul tuturor actualizărilor, vom avea nevoie de o altă rutină ajutătoare, care să ne afișeze conținutul final din rezervor.
*  Vom apela programul cu opțiunea -o pentru a invoca această rutină de afișare finală.
*/

void secventa_de_actualizari(int n, char* nr[]);
/*
*  Rutina principală a programului, care va executa secvența de actualizări primită în linia de comandă, în manieră concurentă,
*  conform celor specificate în enunțul problemei.
*  Observație: rutina principală nu este implementată în întregime mai jos, ci vă rămâne dvs. sarcina să completați "bucățile" de cod
*  care lipsesc (indicate prin comentarii TODO).
*/

int main(int argc, char* argv[])
{
	if(argc == 1)
	{
		fprintf(stderr, "Eroare: programul trebuie apelat cu optiunile: -i, -o, sau cu o secventa de numere reale...\n");  return 1;
	}

	/* Cazul de inițializare a fișierului de date. */
	if( strcmp(argv[1],"-i") == 0 )
	{
		initializare_fisier_date();  return 0;
	}

	/* Cazul de afișare a fișierului de date. */
	if( strcmp(argv[1],"-o") == 0 )
	{
		afisare_fisier_date();  return 0;
	}

	/* Cazul general: actualizarea fișierului de date. */
	secventa_de_actualizari(argc, argv);
	return 0;
}


void initializare_fisier_date()
{/* Funcționalitate: inițializarea fișierului de date. */

	int fd;
	float val;

	printf("Dati cantitatea initiala de combustibil (i.e., un numar real pozitiv): ");
	while( (1 != scanf("%f", &val)) && (val < 0) ) { fprintf(stderr,"Eroare: nu ati introdus un numar real pozitiv! Incercati din nou..."); }

	if(-1 == (fd = creat("peco.bin", 0600) ) )
	{
		perror("Eroare la crearea fisierului de date...");  exit(2);
	}

	if(-1 == write(fd, &val, sizeof(float) ) )
	{
		perror("Eroare la scrierea valorii initiale in fisierul de date...");  exit(3);
	}

	close(fd);
}


void afisare_fisier_date()
{/* Funcționalitate: afișarea fișierului de date. */

	int fd;
	float val;

	if ( -1 == (fd = open("peco.bin", O_RDONLY) ) )
	{
		perror("Eroare la deschiderea pentru afisare a fisierului de date...");  exit(4); 
	}

	if (-1 == read(fd, &val, sizeof(float) ) )
	{
		perror("Eroare la citirea valorii finale din fisierul de date...");  exit(5); 
	}

	close(fd);
	printf("Stocul final de combustibil este: %f litri de combustibil.\n", val);
}


void secventa_de_actualizari(int n, char* nr[])
{ /* Funcționalitate: realizarea secvenței de operații de actualizare a fișierului de date. */

	float valoare, stoc;
	int fd,i;
    struct flock lacat;

	if ( -1 == (fd = open("peco.bin", O_RDWR) ) )
	{
		perror("Eroare la deschiderea pentru actualizare a fisierului de date...");  exit(6); 
	}

	srandom( getpid() ); //  Inițializarea generatorului de numere aleatoare.

	for(i = 1; i < n; i++) // Iterăm prin secvența de numere reale specificată în linia de comandă.
	{
		if(1 != sscanf(nr[i], "%f", &valoare))  // Facem conversie de la string la float.
		{
			fprintf(stderr, "[PID: %d] Eroare: ati specificat o valoare invalida drept argument de tip numar real : %s !\n", getpid(), nr[i] );
			exit(7);
		}

        lacat.l_type = F_WRLCK;
        lacat.l_whence = SEEK_SET;
        lacat.l_start = 0;
        lacat.l_len = 1;    

        printf("\n[ProcesID:%d] Incep incercarile de punere a blocajului prin apel neblocant.\n",getpid());


		////////////////////////

        int codBlocaj;
        while( (-1 == (codBlocaj = fcntl(fd,F_SETLK,&lacat)) ) && ((errno == EACCES)||(errno == EAGAIN)) )
        { 
            fprintf(stderr, "[ProcesID:%d] Asteptam punerea lacatului pt write...", getpid());
            perror("\tMotivul erorii");
            sleep(2); // O mică pauză, înainte de a reîncerca punerea lacătului.
        }
        if(codBlocaj == -1)
        {
            fprintf(stderr, "[ProcesID:%d] Eroare grava la blocaj...", getpid());
            perror("\tMotivul erorii");
            exit(3);
        }
    


		if(-1 == lseek(fd, 0, SEEK_SET) ) // (Re)poziționăm cursorul la începutul fișierului.
		{
			perror("Eroare la repozitionarea in fisierul de date, pentru citire...");  exit(8); 
		}
		if (-1 == read(fd, &stoc, sizeof(float) ) )
		{
			perror("Eroare la citirea valorii din fisierul de date...");  exit(9); 
		}

		printf("[PID: %d] Se adauga/extrage in/din rezervor cantitatea de %f litri de combustibil.\n", getpid(), valoare );

		
        if(stoc + valoare < 0) 
        {
            fprintf(stderr, "[ProcesID:%d] Stocul este negativ, nu o sa se execute comanda.", getpid());
        }
        else{
		    stoc = stoc + valoare;
        }

		if(-1 == lseek(fd, 0, SEEK_SET) ) // Repoziționăm cursorul la începutul fișierului.
		{
			perror("Eroare la repozitionarea in fisierul de date, pentru scriere...");  exit(10); 
		}
		if (-1 == write(fd, &stoc, sizeof(float) ) )
		{
			perror("Eroare la scrierea valorii actualizate in fisierul de date..."); exit(11); 
		}
  
        lacat.l_type = F_UNLCK;
		if((-1 == (codBlocaj = fcntl(fd,F_SETLK,&lacat)) ) && ((errno == EACCES)||(errno == EAGAIN)))
        {
            perror("Eroare critica la deblocarea fisierului..."); return;
        }
      
		sleep( random() % 5 ); // Facem o pauză aleatoare, de maxim 4 secunde, înainte de a continua cu următoarea iterație.
	}     

	close(fd);
	printf("[PID: %d] Am terminat de procesat secventa de actualizari.", getpid());
}