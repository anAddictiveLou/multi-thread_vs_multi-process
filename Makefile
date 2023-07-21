all:
	gcc -o multi_thread multi_thread.c -lpthread
	gcc -o multi_process multi_process.c -lrt