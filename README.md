# multi-thread_vs_multi-process

# Purpose:
This program counts number of prime number in given range.__
You can specific how many threads and process can be used for calculation.__
For example:__ 
  in range 1 - n with 1 thread: thread1 1 - n__
                 with 2 threads:thread1 1 - n/2; thread2 n/2+1 - n__
                 with 3 threads: thread  1 - n/3; thread2 n/3+1 - 2n/3; thread3 2n/3+1 - n__ 
  and so on...__

# How to use
  make
  time ./multi_thread <bg> <end> <number_of_thread>

# Note
 time command for tracing time executing of program
