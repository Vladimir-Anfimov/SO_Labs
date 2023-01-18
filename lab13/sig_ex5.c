    /*
*  Program: sig_ex5.c
*
*  Funcționalitate: ilustrează modul de așteptare a unui anumit tip de semnal.
*/

#include <stdio.h>
#include <signal.h>

int main()
{
  sigset_t mask;

  /* Stabilește masca de semnale ce vor fi blocate: toate semnalele, exceptând SIGQUIT (și tipurile de semnale neblocabile) */
  sigfillset(&mask);
  sigdelset (&mask, SIGQUIT);

  printf("Suspendarea executiei programului pana la apasarea tastelor CTRL+\\.\n");

  /* Se suspendă execuția programului, în așteptarea sosirii unui semnal SIGQUIT, restul semnalelor fiind blocate (cu excepția celor neblocabile) */
  sigsuspend(&mask);

  printf("Sfarsit program.\n");
  return 0;
}