#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

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
    printf("thread id: %ld, bg: %d, end: %d, count = %d\n", pthread_self(), temp->bg, temp->end, temp->count);
}

int main(int argc, char** argv) {
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <bg> <end> <num_of_threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    else
    {
        struct Prime total_prime = {.bg = atoi(argv[1]), .end = atoi(argv[2]), .count = 0};
        int num_of_threads = atoi(argv[3]);
        int range = total_prime.end - total_prime.bg + 1;
        int interval = range / num_of_threads;
        int last_interval = range % num_of_threads;
        struct Prime* thread_prime = (struct Prime*) calloc(num_of_threads, sizeof(struct Prime));
        pthread_t* thread_id = (pthread_t*) calloc(num_of_threads, sizeof(pthread_t));  
        
        for (int i = 0; i < num_of_threads; i++)
        {
            if (i == num_of_threads - 1 && 0 != last_interval)
            {
                thread_prime[i].bg = thread_prime[i-1].end + 1;
                thread_prime[i].end = total_prime.end;
            }
            else 
            {
                thread_prime[i].bg = ((0 == i) ? total_prime.bg : thread_prime[i-1].end+1);
                thread_prime[i].end = total_prime.bg + (i + 1) * interval - 1;
            }            
            if (pthread_create(&thread_id[i], NULL, count_prime, (void*) &thread_prime[i]) != 0) 
            {
                perror("Error creating thread");
                return 1;
            } 
        }
        for (int i = 0; i < num_of_threads; i++)
        {
            if (pthread_join(thread_id[i], NULL) != 0) 
            {
                perror("Error joining thread");
                return 1;
            }
            total_prime.count+=thread_prime[i].count;
        }
        printf("KQ = %d, ", total_prime.count);
        exit(EXIT_SUCCESS);
    }
}
