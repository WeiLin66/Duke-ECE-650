#include "my_malloc.h"

/**
 * first fit malloc 
 */
void *ff_malloc(size_t size) {
    void *p = sbrk(0);
    void *request = sbrk(size);

    if (request == (void*) -1) {
        return NULL;
    }

    assert(p == request);
    return p;   
}


/**
 * test function 
 */
int main(int argc, char*argv[]){
    // test code here

    return 0;
}