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
#include <signal.h>

struct Prime {
    long long bg;
    long long end;
    long long count;
};

volatile long long catch = 0;

void sigchld_handler() 
{
    int status;
    pid_t child_pid;
    while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0) 
    {
        catch++;
    }
}

bool isPrime(long long num) {
    if (num <= 1)
        return false;
    if (num <= 3)
        return true;
    if (num % 2 == 0 || num % 3 == 0)
        return false;
    for (long long i = 5; i * i <= num; i += 6) {
        if (num % i == 0 || num % (i + 2) == 0)
            return false;
    }
    return true;
}

void* count_prime(void* arg) 
{
    struct Prime* temp = (struct Prime*) arg;
    for (long long i = temp->bg; i <= temp->end; i++)
    {
        if (true == isPrime(i))
            temp->count++;
    }
    //printf("process id: %d, bg: %lld, end: %lld, count = %lld\n", getpid(), temp->bg, temp->end, temp->count);
}

int main(int argc, char** argv) 
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <bg> <end> <num_of_processes>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    else
    {
        /* ignore SIGCHLD handler*/
        sigset_t block_mask, empty_mask;
        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sa.sa_handler = sigchld_handler;
        sigaction(SIGCHLD, &sa, NULL);

        sigemptyset(&block_mask);
        sigaddset(&block_mask, SIGCHLD);
        sigprocmask(SIG_SETMASK, &block_mask, NULL);

        /* Set up shared mem */
        char* name = "shared mem";
        int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
        long long num_of_processes = atoi(argv[3]);
        long long total_prime = num_of_processes;
        long long size_sharing = sizeof(struct Prime) * num_of_processes + 1;
        ftruncate(shm_fd, size_sharing);
        struct Prime* process_prime = (struct Prime*) mmap(0, size_sharing, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0);
        memset(process_prime, 0, size_sharing);

        process_prime[total_prime].bg = atoi(argv[1]);
        process_prime[total_prime].end = atoi(argv[2]);
        long long range = process_prime[total_prime].end - process_prime[total_prime].bg + 1;
        long long interval = range / num_of_processes;
        long long last_interval = range % num_of_processes;
    
        long long success_fork = 0;
        for (long long i = 0; i < num_of_processes; i++)
        {
            if (i == num_of_processes - 1 && 0 != last_interval)
            {
                process_prime[i].bg = process_prime[i-1].end + 1;
                process_prime[i].end = process_prime[total_prime].end;
            }
            else 
            {
                process_prime[i].bg = ((0 == i) ? process_prime[total_prime].bg : process_prime[i-1].end+1);
                process_prime[i].end = process_prime[total_prime].bg + (i + 1) * interval - 1;
            }      
            switch(fork())
            {
                case -1:
                    fprintf(stderr, "fork() failed!");
                    exit(EXIT_FAILURE);
                case 0:
                    success_fork++;
                    count_prime((void*)&process_prime[i]);
                    process_prime[total_prime].count+=process_prime[i].count;
                    exit(EXIT_SUCCESS);
                default:
                    break;
            }
        }
        sigemptyset(&empty_mask);
        while (catch < num_of_processes)
        {
            sigsuspend(&empty_mask);
        }
        printf("%lld, ", num_of_processes);
        //printf("%lld\n", process_prime[total_prime].count);
        shm_unlink(name);

        exit(EXIT_SUCCESS);
    }
}
