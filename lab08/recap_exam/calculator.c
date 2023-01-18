#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#define FILE_NAME "calculator_data.bin"
#define MAX_VALUE 10
#define NR_OPERANZI 4

#define handle_error(msg) \
    {perror(msg); exit(EXIT_FAILURE);}

void generate_random_ch1(int N, int *map_addr);
void generate_random_ch2(int N, char *map_addr);

pid_t Fork()
{
    pid_t pid = fork();
    if(pid == -1) handle_error("Eroare la fork");
    return pid;
}

int main(int argc, char **argv)
{
    int N, fd, file_size;
    void *map_addr;
    
    if(argc < 2)
        fprintf(stderr, "Apel: %s N\n", argv[0]), exit(EXIT_FAILURE);

    N = atoi(argv[1]);

    if(-1 == (fd = open(FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0600))) 
        handle_error("Eroare la open FILE_NAME in generator");

    file_size = N*2*sizeof(int) + N*sizeof(char);

    if(-1 == ftruncate(fd, file_size))
        handle_error("Eroare la ftruncate");

    if(MAP_FAILED == (map_addr = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)))
        handle_error("Eroare la mmap");

    srand(getpid());


    if(0 == Fork())
    {
        generate_random_ch1(N, map_addr);
        exit(EXIT_SUCCESS);
    }

    if(0 == Fork())
    {
        generate_random_ch2(N, map_addr);
        exit(EXIT_SUCCESS);
    }
    
    wait(NULL); wait(NULL);

    int numberOne, numberTwo, offsetInt = 0, offsetChar = 2*N*sizeof(int);
    char operand;



    for(int i=0;i<N;i++)
    {
        numberOne = ((int*)map_addr)[offsetInt++];
        numberTwo = ((int*)map_addr)[offsetInt++];
        operand = ((char*)map_addr)[offsetChar++];
        switch (operand)
        {
        case '+':
            printf("%d + %d = %d\n", numberOne, numberTwo, numberOne+numberTwo);
            break;
        case '-':
            printf("%d - %d = %d\n", numberOne, numberTwo, numberOne-numberTwo);
            break;
        case '*':
            printf("%d * %d = %d\n", numberOne, numberTwo, numberOne*numberTwo);
            break;        
        case '/':
        {
            if(numberTwo == 0) printf("%d / %d = IMPOSIBIL\n", numberOne, numberTwo);
            else printf("%d / %d = %d\n", numberOne, numberTwo, numberOne/numberTwo);
            break;               
        }
        default:
            printf("Parametrii %d, %d, %c invalizi\n", numberOne, numberTwo, operand);
            break;
        }
    }

    if(-1 == munmap(map_addr, file_size))
        handle_error("Eroare la munmap");

    return 0;
}

void generate_random_ch1(int N, int* map_addr)
{
    
    int offset = 0;
    for(int i=0;i<N*2;i++)
    {
        int number = rand() % MAX_VALUE;
        map_addr[offset++] = number;
    }
}

void generate_random_ch2(int N, char *map_addr)
{
    char operanzi[5] = {'+', '-', '*', '/'};
    int offset = sizeof(int) * N * 2;
    for(int i=0;i<N;i++)
    {
        int index_operand = rand() % NR_OPERANZI;
        map_addr[offset++] = operanzi[index_operand];
    }    
}