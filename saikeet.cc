#include <iostream>
#include <pthread.h>

#include "saikeet.hh"

void hello_world()
{
    std::cout << "1 - Hello, world!" << std::endl;
    std::cout << "2 - It's a wonderful world!" << std::endl;
    std::cout << "3 - Goodbye world!" << std::endl;
}

int main()
{
    // create 3 threads and run hello_world() in each of them
    pthread_t thread1, thread2, thread3;
    pthread_create(&thread1, NULL, (void *(*)(void *))hello_world, NULL);
    pthread_create(&thread2, NULL, (void *(*)(void *))hello_world, NULL);
    pthread_create(&thread3, NULL, (void *(*)(void *))hello_world, NULL);

    // wait for the threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    return 0;
}
