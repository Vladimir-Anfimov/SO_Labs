#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#define NR_AFISARI 100  /* Câte perechi de mesaje "hi-ho, " dorim să fie afișate. */

void handler_sigusr1(int nr_sem)
{
	fprintf(stderr,"Debug: procesul cu PID-ul %d a primit semnalul de notificare.\n", getpid());
	/*
		Notă: acesta este doar un mesaj cu rol de debugging; se poate "ignora" la execuție prin redirectarea stderr:
		UNIX> ./a.out  2> /dev/null
	*/
}

int main()
{
	int pid,i;
	sigset_t masca_semnale_usr1_blocat, masca_semnale_usr1_deblocat;

	/* Configurăm masca curentă de semnale, astfel încât SIGUSR1 să fie blocat. */
	sigemptyset(&masca_semnale_usr1_blocat);
	sigaddset(&masca_semnale_usr1_blocat,SIGUSR1);

	if(-1 == sigprocmask(SIG_BLOCK,&masca_semnale_usr1_blocat,NULL) )
	{
		perror("Eroare la sigprocmask");  exit(2);
	}

	/* Pregătim masca alternativă de semnale, ce va fi activată doar pe durata apelurilor sigsuspend din buclele for de mai jos. */
	sigfillset(&masca_semnale_usr1_deblocat);
	sigdelset(&masca_semnale_usr1_deblocat,SIGUSR1);

	/* Corupem semnalul SIGUSR1. Notă: o putem face și după apelul fork, dar imediat după el (i.e., înainte de if-ul de ramificare!) */
	if(SIG_ERR == signal(SIGUSR1,handler_sigusr1) )
	{
		perror("Eroare la signal");  exit(1);
	}

	/* Creăm procesul fiu. Notă: NU este esențial ce tip de mesaje va afișa el (HI-uri sau HO-uri), ci doar ca tatăl să afișeze celălalt tip de mesaje. */
	if(-1 == (pid=fork()) )
	{
		perror("Eroare la fork");  exit(3);
	}

	if(pid == 0)
	{ /* Fiul : alegem să fie responsabil cu afișarea HI-urilor... */
	
		for(i = 0; i < NR_AFISARI; i++)
		{
			printf("hi-");  fflush(stdout);            /* Afișez pe ecran mesajul HI. */

			kill(getppid(),SIGUSR1);                   /* Trimit notificare către celălalt proces. */

			sigsuspend(&masca_semnale_usr1_deblocat);  /* Aștept notificarea de la celălalt proces. */
		}
	}
	else
	{ /* Tatăl : ca urmare a alegerii de mai sus, acesta va fi responsabil cu afișarea HO-urilor... */
	
		for(i = 0; i < NR_AFISARI; i++)
		{
			sigsuspend(&masca_semnale_usr1_deblocat);  /* Aștept notificarea de la celălalt proces. */

			printf("ho, ");  fflush(stdout);           /* Afișez pe ecran mesajul HO. */

			kill(pid,SIGUSR1);                         /* Trimit notificare către celălalt proces. */
		}

		printf("\nSfarsit.\n");
	}
    
	return 0;
}