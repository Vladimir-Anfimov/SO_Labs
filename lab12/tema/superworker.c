#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#define handle_error(msg) \
    {fprintf(stderr, "Eroare: %s, errno=%d\n", msg, errno);  perror("Cauza erorii");  exit(1);}

int canal_P0toP1[2], canal_P1toP2[2], canal_P2toP0[2]; 

int main()
{
    pid_t pid1, pid2;
    master_init();

    if(-1 == (pid1=fork()) )
        handle_error("la crearea primului fiu");
    if(pid1 == 0)
    {
        // copil 1
        //primul fiu va elimina toate vocalele din textul primit și va transmite textul astfel obținut către fiul al doilea;
        if (close(canal_P0toP1[1]) == -1) handle_error("inchidere write canal_P0toP1");
        
        while (1) {
                char buf;
                int numRead = read(canal_P0toP1[0], &buf, sizeof(char));
                if (numRead == -1) handle_error("la citirea din canal_P0toP1");
                if (numRead == 0) break;
                if(strchr("aeiouAEIOU", buf) != NULL)
                {
                    if (write(canal_P1toP2[1], &buf, numRead) != numRead){
                        handle_error("la scrierea in canal_P1toP2");
                    }
                }
            }
            if (close(canal_P0toP1[0]) == -1) handle_error("inchidere read canal_P0toP1");
            if (close(canal_P1toP2[1]) == -1) handle_error("inchidere write canal_P1toP2");  
    }
    else {
        if(-1 == (pid2=fork()))
            handle_error("la crearea celui de al doilea fiu");
        if(pid2 == 0)
        {
            // copil 2
            //al doilea fiu va procesa textul primit transformând toate literele mici în litere mari, iar rezultatul îl va trimite către tată; 
            if (close(canal_P1toP2[1]) == -1) handle_error("inchidere write canal_P1toP2");
            
            while (1) {
                char buf;
                int numRead = read(canal_P1toP2[0], &buf, sizeof(char));
                if (numRead == -1) handle_error("la citirea din canal_P1toP2");
                if (numRead == 0) break;
                buf = tolower(buf);
                if (write(canal_P2toP0[1], &buf, numRead) != numRead){
                    handle_error("la scrierea in canal_P2toP0");
                }
                
            }
            if (close(canal_P1toP2[0]) == -1) handle_error("inchidere read canal_P1toP2"); 
            if (close(canal_P2toP0[1]) == -1) handle_error("inchidere write canal_P2toP0");  
        }
        else {
            // parinte
            //procesul părinte citește de la tastatură o propoziție, pe care o va transmite primului fiu; 
             //la final, tatăl va afișa pe ecran textul prelucrat primit de la al doilea fiu. 
            if (close(canal_P0toP1[0]) == -1) handle_error("inchidere read canal_P0toP1");           
            char input[100];
            scanf("%s", input);

            for(int i =0;i<strlen(input);i++)
            {
                if (write(canal_P0toP1[1], input[i], sizeof(char)) != sizeof(char)) 
                    handle_error("scriere partiala in canal canal_P0toP1");     
            }
            if (close(canal_P0toP1[1]) == -1) handle_error("inchidere write canal_P0toP1");  
            char output[100];
            int size = 0;
            while (1) {
                char buf;
                int numRead = read(canal_P2toP0[0], &buf, sizeof(char));
                if (numRead == -1) handle_error("la citirea din canal_P1canal_P2toP0toP2");
                if (numRead == 0) break;
                output[size++] = buf;
            }
            output[size] = '\0';
            if (close(canal_P2toP0[0]) == -1) handle_error("inchidere read canal_P2toP0");  
        }
    }
    return 0;
}

void master_init()
{
    if( pipe(canal_P0toP1) == -1 ) handle_error("la crearea canalului canal_P0toP1");
    if( pipe(canal_P1toP2) == -1 ) handle_error("la crearea canalului canal_P1toP2");
    if( pipe(canal_P2toP0) == -1 ) handle_error("la crearea canalului canal_P2toP0");
    
}