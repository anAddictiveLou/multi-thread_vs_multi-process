#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define SIZE 4096

struct Prime {
    int bg;
    int end;
    int count;
};

bool isPrime(int num) {
    if (num <= 1)
        return false;
    if (num <= 3)
        return true;
    if (num % 2 == 0 || num % 3 == 0)
        return false;
    for (int i = 5; i * i <= num; i += 6) {
        if (num % i == 0 || num % (i + 2) == 0)
            return false;
    }
    return true;
}

void* count_prime(void* arg) 
{
    struct Prime* temp = (struct Prime*) arg;
    for (int i = temp->bg; i <= temp->end; i++)
    {
        if (true == isPrime(i))
            temp->count++;
    }
    printf("process id: %d, bg: %d, end: %d, count = %d\n", getpid(), temp->bg, temp->end, temp->count);
}

int main(int argc, char** argv) {
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <bg> <end> <num_of_processes>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    else
    {
        struct Prime total_prime = {.bg = atoi(argv[1]), .end = atoi(argv[2]), .count = 0};
        int num_of_processes = atoi(argv[3]);
        int range = total_prime.end - total_prime.bg + 1;
        int interval = range / num_of_processes;
        int last_interval = range % num_of_processes;
        int* process_id = (int*) calloc(num_of_processes, sizeof(int));  
    
        /* Set up shared mem */
        char* name = "shared mem";
        int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
        int size_sharing = sizeof(struct Prime) * num_of_processes;
        ftruncate(shm_fd, size_sharing);
        struct Prime* process_prime = (struct Prime*) mmap(0, size_sharing, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);

        int success_fork = 0;
        for (int i = 0; i < num_of_processes; i++)
        {
            switch(fork())
            {
                case -1:
                    fprintf(stderr, "fork() failed!");
                    exit(EXIT_FAILURE);
                case 0:
                    success_fork++;
                    if (i == num_of_processes - 1 && 0 != last_interval)
                    {
                        process_prime[i].bg = process_prime[i-1].end + 1;
                        process_prime[i].end = total_prime.end;
                    }
                    else 
                    {
                        process_prime[i].bg = ((0 == i) ? total_prime.bg : process_prime[i-1].end+1);
                        process_prime[i].end = total_prime.bg + (i + 1) * interval - 1;
                    }      
                    count_prime((void*)&process_prime[i]);
                    exit(EXIT_SUCCESS);
                default:
                    break;
            }
        }
        for (int i = 0; i < success_fork; i++)
        {
            wait(NULL);
            total_prime.count+=process_prime[i].count;
        }
        printf("%d\n", total_prime.count);
        exit(EXIT_SUCCESS);
    }
}
