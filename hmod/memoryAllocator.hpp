//
// Created by os on 5/3/24.
//

#ifndef PROJECT_BASE_MEMORYALLOCATOR_HPP
#define PROJECT_BASE_MEMORYALLOCATOR_HPP
#include "../lib/hw.h"

//odrzavacemo listu slobodnih fragmenata
struct BlockHeader{
    uint64 size; //velicina bloka (od adrese do kraja ili do narednog bloka)
    BlockHeader* next; //sledeci blok
};

class MemoryAllocator {
public:
    static void* memAlloc(size_t size);
    static int memFree(void*);
    static void initMem(); //inicijalizacija memorije
private:
    MemoryAllocator(){}
    static uint64 constexpr HEADER_SIZE=sizeof(BlockHeader); //velicina zaglavlja
    static BlockHeader* freeMemHead; //pokazivac na slobodne blokove memorije
    //za spajanje slobodnih blokova
    static void merge(BlockHeader* block1, BlockHeader* block2);
};


#endif //PROJECT_BASE_MEMORYALLOCATOR_HPP
