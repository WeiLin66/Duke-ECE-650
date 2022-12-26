#ifndef __MY_MALLOC_H_
#define __MY_MALLOC_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>

/* First Fit malloc/free */
void* ff_malloc(size_t size);
void ff_free(void* ptr);

/* Best Fit malloc/free */
void* bf_malloc(size_t size);
void bf_free(void* ptr);





#endif /* __MY_MALLOC_H_ */