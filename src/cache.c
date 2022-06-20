//========================================================//
//  cache.c                                               //
//  Source file for the Cache Simulator                   //
//                                                        //
//  Implement the I-cache, D-Cache and L2-cache as        //
//  described in the README                               //
//========================================================//

#include "cache.h"
#include <math.h>
#include <stdio.h>
//
// TODO:Student Information
//
const char *studentName = "Chi-Hsin Lo";
const char *studentID   = "A53311981";
const char *email       = "c2lo@eng.ucsd.edu";

//------------------------------------//
//        Cache Configuration         //
//------------------------------------//

uint32_t icacheSets;     // Number of sets in the I$
uint32_t icacheAssoc;    // Associativity of the I$
uint32_t icacheHitTime;  // Hit Time of the I$

uint32_t dcacheSets;     // Number of sets in the D$
uint32_t dcacheAssoc;    // Associativity of the D$
uint32_t dcacheHitTime;  // Hit Time of the D$

uint32_t l2cacheSets;    // Number of sets in the L2$
uint32_t l2cacheAssoc;   // Associativity of the L2$
uint32_t l2cacheHitTime; // Hit Time of the L2$
uint32_t inclusive;      // Indicates if the L2 is inclusive

uint32_t blocksize;      // Block/Line size
uint32_t memspeed;       // Latency of Main Memory

//------------------------------------//
//          Cache Statistics          //
//------------------------------------//

uint64_t icacheRefs;       // I$ references
uint64_t icacheMisses;     // I$ misses
uint64_t icachePenalties;  // I$ penalties

uint64_t dcacheRefs;       // D$ references
uint64_t dcacheMisses;     // D$ misses
uint64_t dcachePenalties;  // D$ penalties

uint64_t l2cacheRefs;      // L2$ references
uint64_t l2cacheMisses;    // L2$ misses
uint64_t l2cachePenalties; // L2$ penalties

//------------------------------------//
//        Cache Data Structures       //
//------------------------------------//

//
//TODO: Add your Cache data structures here
//

//doubly linked list
typedef struct block{
    struct block *prev, *next;
    uint32_t value;// tag
}block;

typedef struct set{
    block *head;
    uint32_t size;
    uint32_t capacity;
}set;

set *Icache, *Dcache, *L2cache;
//------------------------------------//
//          Cache Functions           //
//------------------------------------//

block* initBlock(uint32_t aValue){
    block* newBlock = (block*)malloc(sizeof(block));
    newBlock->value = aValue;
    newBlock->prev = newBlock;
    newBlock->next = newBlock;
    return newBlock;
}

int add_block(set *aSet, uint32_t value){
    //printf("add block value %d \n", value);
    block* theBlock = initBlock(value);
    if( aSet->size == 0 ){
        aSet->head = theBlock;
        aSet->size = 1;
        return -1;
    }
    else{ //add block to the front of the set
        // block    head->prev head head->next
        theBlock->next = aSet->head;
        theBlock->prev = aSet->head->prev;
        aSet->head->prev->next = theBlock;
        aSet->head->prev = theBlock;
        aSet->head = theBlock;
        aSet->size += 1;
        
        if(aSet->size > aSet->capacity){ //evict last block
            block *to_evict = aSet->head->prev;
            uint32_t evicted = to_evict->value;
            
            aSet->head->prev = to_evict->prev;
            to_evict->prev->next = aSet->head;
            
            free(to_evict);
            aSet->size = aSet->capacity;
            
            return evicted;
        }
        else {
            return -1;
        }
    }
    
}

// return 1 if the value is found and move to front
int found(set *aSet, uint32_t aValue){
    if (aSet->size == 0 || aSet->capacity == 0 || aSet->head == NULL){
        //printf("return 0 ");
        return 0;
    }
    else if (aSet->size == 1){
        return aSet->head->value == aValue;
    }
    
    block *theHead = aSet->head;
    block *tmp = theHead;
    do{
        if(tmp->value == aValue){
            break;
        }
        tmp = tmp->next;
    }
    while(tmp != theHead);
    
    
    if(tmp->value == aValue){
        //printf("found \n");
        if(tmp->next == theHead){
            aSet->head = tmp;
        }
        else if(tmp != theHead){
            // move tmp to head
            tmp->next->prev = tmp->prev;
            tmp->prev->next = tmp->next;
            
            tmp->prev = theHead->prev;
            tmp->next = theHead;
            theHead->prev->next = tmp;
            theHead->prev = tmp;
            
            aSet->head = tmp;
        }
        return 1;
    }
    //printf("return 0 ");
    return 0;
}

void print_set(set *aSet){
    //printf("\n set size = %d, set capacity = %d, tags = ", aSet->size, aSet->capacity);
    if(aSet->size == 0) return;
    block *theHead = aSet->head;
    block *tmp = theHead;

    do{
        printf("%d ", tmp->value);
        tmp = tmp->next;
    }
    while(tmp != theHead);
    
}

int evict_head(set *aSet){
    if (aSet->size == 0 || aSet->capacity == 0 ){
        return 0;
    }
    else if(aSet->size == 1){
        free(aSet->head);
        aSet->head = NULL;
        aSet->size = 0;
    }
    else{
        block *tmp = aSet->head;
        aSet->head = tmp->next;
        tmp->prev->next = tmp->next;
        tmp->next->prev = tmp->prev;
        free(tmp);
        aSet->size -= 1;
    }
    return 1;
}

// Initialize the Cache Hierarchy
//
void
init_cache()
{
  // Initialize cache stats
  icacheRefs        = 0;
  icacheMisses      = 0;
  icachePenalties   = 0;
  dcacheRefs        = 0;
  dcacheMisses      = 0;
  dcachePenalties   = 0;
  l2cacheRefs       = 0;
  l2cacheMisses     = 0;
  l2cachePenalties  = 0;
  
  //
  //TODO: Initialize Cache Simulator Data Structures
  //
    
  Icache = (set*)malloc( icacheSets * sizeof(set) );
  for(int i=0; i<icacheSets; i++){
      Icache[i].capacity = icacheAssoc;
      Icache[i].size = 0;
      Icache[i].head = NULL;
      //print_set(&Icache[i]);
  }
    
  Dcache = (set*)malloc( dcacheSets * sizeof(set) );
  for(int i=0; i<dcacheSets; i++){
      Dcache[i].capacity = dcacheAssoc;
      Dcache[i].size = 0;
      Dcache[i].head = NULL;
      //print_set(&Dcache[i]);
  }
    
  L2cache = (set*)malloc( l2cacheSets * sizeof(set) );
  for(int i=0;i <l2cacheSets; i++){
      L2cache[i].capacity = l2cacheAssoc;
      L2cache[i].size = 0;
      L2cache[i].head = NULL;
      //print_set(&L2cache[i]);
  }
    
}


// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t icache_access(uint32_t addr)
{
    //printf("\n icache access");
    // return if cache size 0
    if (icacheSets == 0){
        uint32_t time = l2cache_access(addr);
        icachePenalties += time;
        return time;
    }
    
    // add 1 to ichache reference
    icacheRefs += 1;
    
    // block offset
    uint32_t offsetBits = log2(blocksize);
    uint32_t offset = addr & ( (1<<offsetBits)-1 );
    
    // index
    uint32_t indexBits = log2(icacheSets);
    uint32_t index = (addr >> offsetBits) & ( (1<<indexBits)-1 );
    
    // tag
    uint32_t tag = (addr >> (offsetBits + indexBits));
        
    /*
    printf("\n before found : ");
    print_set(&Icache[index]);*/
    if( found(&Icache[index], tag) == 1 ){
        /*
        printf("\n after found : ");
        print_set(&Icache[index]);*/
        return icacheHitTime;
    }
    else{ // not found in icache
        icacheMisses += 1;
        uint32_t time = l2cache_access(addr);
        icachePenalties += time;
        /*
        printf("\n before add block : ");
        print_set(&Icache[index]);*/
        add_block(&Icache[index], tag);
        /*
        printf("\n after add block : ");
        print_set(&Icache[index]);*/
        
        return icacheHitTime + time;
    }
}

// Perform a memory access through the dcache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t dcache_access(uint32_t addr)
{
    //printf("\n dcache access");
    // return if cache size 0
    if (dcacheSets == 0){
        uint32_t time = l2cache_access(addr);
        dcachePenalties += time;
        return time;
    }
    
    // add 1 to ichache reference
    dcacheRefs += 1;
    
    // block offset
    uint32_t offsetBits = log2(blocksize);
    uint32_t offset = addr & ( (1<<offsetBits)-1 );
    
    // index
    uint32_t indexBits = log2(dcacheSets);
    uint32_t index = (addr >> offsetBits) & ( (1<<indexBits)-1 );
    
    // tag
    uint32_t tag = (addr >> (offsetBits + indexBits));
        
    /*
    printf("\n before found : ");
    print_set(&Dcache[index]);*/
    if( found(&Dcache[index], tag) == 1){
        /*
        printf("\n after found : ");
        print_set(&Dcache[index]);*/
        return dcacheHitTime;
    }
    else{ // not found in icache
        //printf("\n dcache miss");
        //printf("\n %d ",found(&Dcache[index], tag));
        dcacheMisses += 1;
        uint32_t time = l2cache_access(addr);
        dcachePenalties += time;
        //printf("\n before add block : ");
        //print_set(&Dcache[index]);
        add_block(&Dcache[index], tag);
        //printf("\n after add block : ");
        //print_set(&Dcache[index]);
        
        
        return dcacheHitTime + time;
    }
}

// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
//
uint32_t l2cache_access(uint32_t addr)
{
    //printf("\n l2cache access");
    if (l2cacheSets == 0){
        l2cachePenalties += memspeed;
        return memspeed;
    }
    
    l2cacheRefs += 1;
    
    // block offset
    uint32_t offsetBits = log2(blocksize);
    uint32_t offset = addr & ( (1<<offsetBits)-1 );
    
    // index
    uint32_t indexBits = log2(l2cacheSets);
    uint32_t index = (addr >> offsetBits) & ( (1<<indexBits)-1 );
    
    // tag
    uint32_t tag = (addr >> (offsetBits + indexBits));
       
    // find in L2cache
    if( found(&L2cache[index], tag) == 1 ) {
        return l2cacheHitTime;
    }
    else { // not found in L2cache
        l2cacheMisses += 1 ;
        l2cachePenalties += memspeed;
        
        int evicted = add_block(&L2cache[index],  tag);
        /*
        if(evicted == -1){
            return l2cacheHitTime + memspeed;
        }
        else if(inclusive == 1){
            evicted = (evicted << indexBits) + index;
              
            // remove from Icache
            uint32_t I_indexBits = log2(icacheSets);
            uint32_t I_index = evicted & ( (1<<I_indexBits)-1 );
            uint32_t I_tag = (evicted >> I_indexBits);
            if( found(&Icache[I_index], I_tag) == 1 ){
                evict_head(&Icache[I_index]);
            }
      
            // remove from Dcache
            uint32_t D_indexBits = log2(dcacheSets);
            uint32_t D_index = evicted & ( (1<<D_indexBits)-1 );
            uint32_t D_tag = (evicted >> D_indexBits);
            if( found(&Dcache[D_index], D_tag) == 1 ){
                evict_head(&Dcache[D_index]);
            }
        }*/
        return l2cacheHitTime + memspeed;
    }
    
}
