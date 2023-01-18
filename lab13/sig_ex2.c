/*
*  Program: sig_ex2.c
*
*  Funcționalitate: realizează o buclă infinită, cu ignorarea semnalelor de întrerupere sau terminare a execuției programului.
*/

#include <stdio.h>
#include <signal.h>

int main()
{
  /* Setarea ignorării semnalelor SIGINT și SIGQUIT */
  signal( SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);

  printf("Inceput bucla infinita; apasarea tastelor CTRL+C sau CTRL+\\ nu are niciun efect (practic, sunt ignorate...) !\n");

  for(;;);

  printf("Sfarsit program.\n");
  return 0;
}