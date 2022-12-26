#include "my_malloc.h"

META_LIST_INIT(meta_blk_list);


/**
 * Get Virtual Memory form kernel
 */ 
static void get_vm_from_kernel(META_BLK_LIST* list){

    void* get_mem = NULL;

    if(list->head == NULL && list->cur == NULL){

        #if (MY_DEBUG)
            printf("[Info]: First get Virtual Memoey from kernel!\n");
        #endif
        get_mem = sbrk(0);
        list->head = list->tail = get_mem;
    }

    get_mem = sbrk(GET_VM_SIZE);
    assert(get_mem != (void*)-1);
    list->tail += GET_VM_SIZE;

    #if (MY_DEBUG)
        printf("[Info]: Extend Virtual Memory: %d\n", GET_VM_SIZE);
    #endif
}


/**
 * print all dll info
 */ 
static void print_meta_blk_info(){

    #if (MY_DEBUG)
        printf("\n[Info]: ");
        ITERATE_LIST_BEGIN(ptr, GET_META_HEAD)
            printf("[size: %d, is_free: %d] --> ", ptr->data_blk_size, ptr->is_free);
        ITERATE_LIST_END

        printf("NULL\n");

        printf("[Info]: ");
        ITERATE_LIST_REVERSE_BEGIN(ptr, GET_META_CUR)
            printf("[size: %d, is_free: %d] --> ", ptr->data_blk_size, ptr->is_free);
        ITERATE_LIST_END

        printf("NULL\n");
        printf("[Info] Largest Free Segment Size: %lu\n", get_largest_free_data_segment_size());
        printf("[Info] Tatal Free Segment Size: %lu\n\n", get_total_free_size());
    #endif
}


/**
 * split data block when demanded size is smaller than actual data block size
 */ 
static void* split(META_BLK* node, size_t size){

    #if (MY_DEBUG)
        printf("[Info]: Split Block!\n");
    #endif

    uint32_t original_size = node->data_blk_size;
    META_BLK* new_meta = NEXT_SPLIT_META(node, size);

    node->data_blk_size = size;
    new_meta->data_blk_size = original_size - (META_SIZE + size);
    new_meta->is_free = true;
    node->is_free = false;

    BLIND_BLKS_FOR_SPLITING(node, new_meta);

    return (uint8_t*)node + META_SIZE;
}


/**
 * merge free data blocks
 */ 
static void merge(META_BLK* node){

    META_BLK* pre_blk = node->pre;
    META_BLK* next_blk = node->next;
    META_BLK* ret = node;

    uint32_t total_blk_size = 0;
    bool merge = false;

    if(pre_blk == NULL && next_blk == NULL){

        return;
    }

    if(pre_blk != NULL && pre_blk->is_free){

        total_blk_size += (pre_blk->data_blk_size + META_SIZE);
        ret = pre_blk;
        pre_blk = pre_blk->pre;
        merge = true;
    }

    if(next_blk != NULL && next_blk->is_free){

        total_blk_size += (next_blk->data_blk_size + META_SIZE);
        next_blk = next_blk->next;
        merge = true;
    }

    if(!merge){

        return;
    }

    total_blk_size += node->data_blk_size;

    ret->data_blk_size = total_blk_size;
    ret->is_free = true;

    ret->pre = pre_blk;
    if(pre_blk){
        pre_blk->next = ret;
    }

    ret->next = next_blk;
    if(next_blk){
        next_blk->pre = ret;
    }

    #if (MY_DEBUG)
        printf("[Info]: Merge Block!\n");
    #endif
}


/**
 * find any empty block in dll
 */ 
static void* find_empty_blk(size_t size, bool version){

    if(meta_blk_list.cur == NULL){

        return NULL;
    }

    META_BLK* best_fit_ptr = NULL;

    ITERATE_LIST_BEGIN(ptr, GET_META_HEAD)
        if(ptr->is_free){
            
            if(ptr->data_blk_size == size){

                ptr->is_free = false;
                return (uint8_t*)ptr + META_SIZE;
            }else if(!version && ptr->data_blk_size > META_SIZE + size){

                return split(ptr, size);
            }else if(version && ptr->data_blk_size > META_SIZE + size){

                if(!best_fit_ptr){

                    best_fit_ptr = ptr;
                    continue;
                }

                best_fit_ptr = best_fit_ptr->data_blk_size > ptr->data_blk_size ? ptr : best_fit_ptr;
            }
        }
    ITERATE_LIST_END

    if(version && best_fit_ptr){

        return split(best_fit_ptr, size);
    }   

    return NULL;
}


/**
 * memory alllocation func
 */ 
static void* memory_allocation_process(size_t size, MALLOC_VERSION version){

    uint32_t total_blk_length = size + META_SIZE;
    uint32_t curr_blk_length = 0;
    META_BLK* find_empty_blk_res = NULL;

    if(meta_blk_list.head == NULL && meta_blk_list.cur == NULL){

        get_vm_from_kernel(&meta_blk_list);
    }

    if((find_empty_blk_res = find_empty_blk(size, version))){

        return find_empty_blk_res;
    }

    if(meta_blk_list.cur != NULL){

        curr_blk_length = GET_META_CUR->data_blk_size + META_SIZE;
        if(meta_blk_list.cur + curr_blk_length + total_blk_length > meta_blk_list.tail){

            get_vm_from_kernel(&meta_blk_list);
        }
    }

    META_BLK* new_meta = meta_blk_list.cur == NULL ? GET_META_HEAD : (META_BLK*)(meta_blk_list.cur + curr_blk_length);
    memset(new_meta, 0x0, META_SIZE);
    new_meta->data_blk_size = size;

    if(new_meta != GET_META_HEAD){

        new_meta->pre = GET_META_CUR;
        GET_META_CUR->next = new_meta;
        meta_blk_list.cur = (uint8_t*)new_meta;
    }else{

        meta_blk_list.cur = (uint8_t*)GET_META_HEAD;
    }

    return meta_blk_list.cur + META_SIZE;
}


/**
 * memory free func
 */ 
static void memory_free_process(void* addr){

    assert(addr);
    META_BLK* free_target = GET_META_BLK(addr);
    merge(free_target);
} 


/**
 * find the largest free segment in the current dll
 */ 
unsigned long get_largest_free_data_segment_size(){

    uint32_t ret = 0;

    ITERATE_LIST_BEGIN(ptr, GET_META_HEAD)
        if(ptr->is_free){

            ret = ptr->data_blk_size > ret ? ptr->data_blk_size : ret;
        }
    ITERATE_LIST_END

    return ret;
}


/**
 * get total free segment size in the current dll
 */ 
unsigned long get_total_free_size(){

    uint32_t ret = 0;

    ITERATE_LIST_BEGIN(ptr, GET_META_HEAD)
        if(ptr->is_free){

            ret += ptr->data_blk_size;
        }
    ITERATE_LIST_END

    return ret;
}


/**
 * first fit malloc func
 */ 
void* ff_malloc(size_t size){

    return memory_allocation_process(size, First_Fit);
}


/**
 * best fit malloc func
 */ 
void* bf_malloc(size_t size){

    return memory_allocation_process(size, Best_Fit);
}


/**
 * first fit free func
 */ 
void ff_free(void* addr){

    memory_free_process(addr);
}


/**
 * best fit free func
 */ 
void bf_free(void* addr){

    memory_free_process(addr);
}

int main(int argc, char*argv[]){

    uint8_t* ptr1 = ff_malloc(10);
    uint8_t* ptr2 = ff_malloc(20);
    uint8_t* ptr3 = ff_malloc(30);

    print_meta_blk_info();

    return 0;
}