    #include <stdio.h>
    #include <stdlib.h>
    #include <fcntl.h>
    #include <string.h>
    #include <errno.h>

    #define BLOCK_SIZE 4096

    int copiere_rot13(int fdIn, int fdOut);
    // Declaratia functiei urmeaza mai jos dupa main
    
    int main(int argc, char **argv)
    {
        int fdIn, fdOut;
        if(argc != 3)
        {
            fprintf(stderr, "Utilizare %s [intrare-fisier] [iesire-fisier] \n", argv[0]);
            return 1;
        }
        
        printf("Fisierul de intrare este: %s\n", argv[1]);
        if(-1 == (fdIn = open(argv[1], O_RDONLY)))
        {
            perror("Eroare la apelul open \n");
            return 2;
        }

        printf("Fisierul de iesire este: %s\n", argv[2]);
        
        if(0 == access(argv[2], F_OK))
        {
            printf("Fisierul %s exista deja. Doriti sa il suprascrieti? DA/NU \n", argv[2]);
            char raspuns[3];
            while(1)
            {
                scanf("%s", raspuns);
                if(strcmp(raspuns, "DA") == 0)
                {
                    printf("Dreptul de suprascriere a fost acordat. \n");
                    break;
                }
                else if(strcmp(raspuns, "NU") == 0)
                {
                    printf("Dreptul de suprascriere NU a fost acordat. Programul se va inchide fara copiere.\n");
                    return 3;
                }
            }
        }

        if(-1 == (fdOut = creat(argv[2], 0600)))
        {
            perror("Eroare la apelul creat\n");
            return 4;
        }

        return copiere_rot13(fdIn, fdOut);
    }


    int copiere_rot13(int fdIn, int fdOut)
    {
        unsigned char buffer[BLOCK_SIZE];
        int bytes_read;

        while(1)
        {
            bytes_read = read(fdIn, buffer, BLOCK_SIZE);
            if(bytes_read == 0) break;
            if(bytes_read == -1)
            {
                perror("Eroare la citirea din fisier\n");
                return 5;
            }

        for(int i=0; i < bytes_read; i++)
        {
            if('A' <= buffer[i] && buffer[i] <= 'Z')
            {
                buffer[i] = 65 + (buffer[i] - 65 + 13) % 26;
            }
            else if('a' <= buffer[i] && buffer[i] <= 'z')
            {
                buffer[i] = 97 + (buffer[i] - 97 + 13) % 26;
            }
        }

            if(bytes_read != write(fdOut, buffer, bytes_read))
            {
                perror("Eroare la copiere. Numarul de octeti copiati nu corespunde cu cel citit.\n");
                return 6;
            }

        }

        if(-1 == close(fdIn))
        {
            perror("Eroare la inchiderea fisierul din care s-a citit.\n");
            return 6;
        }
        if(-1 == close(fdOut))
        {
            perror("Eroare la inchiderea fisierul in care s-a scris.\n");
            return 7;
        }

        printf("Copierea si procesarea au avut loc cu succes.\n");
        return 0;
    }

