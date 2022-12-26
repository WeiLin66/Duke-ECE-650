#ifndef __MY_MALLOC_H_
#define __MY_MALLOC_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>

#define DEBUG_ON    1
#define DEBUG_OFF   0
#define MY_DEBUG    DEBUG_ON

#define GET_VM_SIZE 1024
#define META_SIZE   sizeof(META_BLK)

#define META_LIST_INIT(list) static META_BLK_LIST list = {NULL, NULL, NULL}

#define ITERATE_LIST_BEGIN(_node, head)         \
        {                                       \
           META_BLK* _node = (META_BLK*)head;   \
           META_BLK* _node_next = NULL;         \
           for(; _node; _node = _node_next){    \
                _node_next = _node->next;


#define ITERATE_LIST_REVERSE_BEGIN(_node, cur) \
        {                                      \
           META_BLK* _node = (META_BLK*)cur;   \
           META_BLK* _node_pre = NULL;         \
           for(; _node; _node = _node_pre){    \
                _node_pre = _node->pre;

#define ITERATE_LIST_END }}

#define BLIND_BLKS_FOR_SPLITING(allocated_meta_block, free_meta_block)      \
            free_meta_block->pre = allocated_meta_block;                    \
            free_meta_block->next = allocated_meta_block->next;             \
            allocated_meta_block->next = free_meta_block;                   \
            if(free_meta_block->next)                                       \
                free_meta_block->next->pre = free_meta_block                
            
#define GET_META_HEAD ((META_BLK*)meta_blk_list.head)
#define GET_META_CUR ((META_BLK*)meta_blk_list.cur)
#define GET_META_TAIL ((META_BLK*)meta_blk_list.tail)

#define GET_DATA_BLK(addr) (META_BLK*)addr + 1
#define GET_META_BLK(addr) (META_BLK*)addr - 1
#define NEXT_SPLIT_META(addr, size) (META_BLK*)((uint8_t*)addr + META_SIZE + size)

typedef struct _META_BLK{

    uint32_t data_blk_size;
    bool is_free;
    struct _META_BLK* pre;
    struct _META_BLK* next;
}META_BLK;

typedef struct _META_BLK_LIST{

    uint8_t* head;
    uint8_t* cur;
    uint8_t* tail;
}META_BLK_LIST;

typedef enum _MALLOC_VERSION{

    First_Fit = 0,
    Best_Fit = 1
}MALLOC_VERSION;

void* ff_malloc(size_t size);
void ff_free(void* addr);
void* bf_malloc(size_t size);
void bf_free(void* addr);
unsigned long get_largest_free_data_segment_size(); //in bytes
unsigned long get_total_free_size(); //in bytes

#endif /* __MY_MALLOC_H_ */