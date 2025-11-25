#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static int glob = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static void* thread_function(void *args)
{
    //cast to convert some types. args pointer is converted to int pointer
    //the * in front of a * is a way to dereference, so you can get the value stored at that address
    int loops = *((int *) args);    //args, we know is the address of &loops
    int ix, ret;
    for(ix = 0; ix < loops; ++ix)
    {
        ret = pthread_mutex_lock(&mtx);
        if(ret != 0) return NULL;

        ++glob; //increment the global variable

        ret = pthread_mutex_unlock(&mtx);
        if(ret != 0) return NULL;
    }

    return NULL;
}
//char pointers are strings
//the first argument in the vector argv[] is the name of the application

int main(int argc, char*argv[])
{
    pthread_t t1, t2;
    int loops, ret;
    //? works like an if statement. if (argc > 1) then return 
    loops = (argc > 1) ? atoi(argv[1]) : 1000000000;   // ? is an operator w/ 3 operands

    ret = pthread_create(&t1, NULL, thread_function, &loops);   //& to give us the address of the loops variable
    ret = pthread_create(&t2, NULL, thread_function, &loops);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("total = %d\n", glob);

    return 0;

}