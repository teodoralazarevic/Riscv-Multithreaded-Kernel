//
// Created by os on 5/3/24.
//

#include "../h/memoryAllocator.hpp"
BlockHeader* MemoryAllocator::freeMemHead= nullptr;

void MemoryAllocator::initMem() {
    MemoryAllocator::freeMemHead=(BlockHeader*) HEAP_START_ADDR;
    //prvi memorijski fragment je ujedno i prvi slobodan fragment, njegov pocetak ce biti
    freeMemHead->size=(char*)HEAP_END_ADDR-(char*)HEAP_START_ADDR-HEADER_SIZE;
    freeMemHead->next= nullptr;
}

//memorija koja ce se zauzeti (zaglavlje+size) mora da bude zaokruzena na blokove
//velicine MEM_BLOCK_SIZE
//parametar funkcije size oznacava broj blokova koji se zahtevaju za alokaciju
//iz memAlloc ce da se vraca prva adresa ispod zaglavlja
//cuva se lista slobodnih fragmenata
void *MemoryAllocator::memAlloc(size_t size) {
    //prostor u blokovima rezervisan za header i potrebnu velicinu
    if(size==0) return nullptr;
    size_t sizeInBytes=size*MEM_BLOCK_SIZE; //koliko bajtova nam je potrebno
    BlockHeader* freeFragment=MemoryAllocator::freeMemHead, *newFragment= nullptr, *prevFragment= nullptr;
    while(freeFragment && freeFragment->size < sizeInBytes){
        prevFragment=freeFragment;
        freeFragment=freeFragment->next;
    }
    if(!freeFragment) return nullptr;
    if(freeFragment->size-sizeInBytes >= MEM_BLOCK_SIZE){ //ako ostaje jos neki slobodan fragment dovoljne velicine
        newFragment=(BlockHeader*)((char*)freeFragment+sizeInBytes);
        newFragment->next=freeFragment->next;
        if(newFragment->next) {
            newFragment->size=(char*)newFragment->next-(char*)newFragment-HEADER_SIZE;
        }
        else {
            newFragment->size=(char*)HEAP_END_ADDR-(char*)newFragment-HEADER_SIZE;
        }
        newFragment->next=freeFragment->next;
    }
    if(prevFragment)
        prevFragment->next=newFragment;
    else
        freeMemHead=newFragment;
    freeFragment->size=sizeInBytes-HEADER_SIZE; //da znamo posle koliko da oslobodimo (velicina prostora ispod header-a)
    freeFragment->next=(BlockHeader*)((char*)freeFragment+HEADER_SIZE);

    void* adr=(char*)freeFragment+HEADER_SIZE;
    return adr; //vracamo prvu adresu ispod zaglavlja
}

int MemoryAllocator::memFree(void* pointer){
    //pointer mora imati neku vrednost vracenu iz mem_alloc, znaci ako od adrese oduzmemo header i konvertujemo
    //u pok na BlockHeader trebamo dobiti odgovarajuce zaglavlje (u polju next bi trebalo da se nalazi bas ovaj
    //pointer ako je prosledjena dobra adresa)
    if(pointer==0) return -1;
    BlockHeader* newBlock=(BlockHeader*)((char*)pointer-HEADER_SIZE);
    BlockHeader* prevBlock= nullptr, *nextBlock= nullptr;
    if(newBlock->next!=pointer) return -1; //nije prosledjena adresa dobijena iz memAlloc
    if((char*)newBlock<HEAP_START_ADDR || (char*)newBlock>HEAP_END_ADDR) return -2; //nevalidna adresa

    if(!freeMemHead){ //ceo prostor je zauzet
        freeMemHead=newBlock;
        return 0;
    }
    if(newBlock<freeMemHead){
        nextBlock=freeMemHead;
        freeMemHead=newBlock;
    }
    else{
        prevBlock=freeMemHead;
        nextBlock=prevBlock->next;
        while(prevBlock && nextBlock && newBlock<prevBlock){
            prevBlock=nextBlock;
            nextBlock=nextBlock->next;
        }
        prevBlock->next=newBlock;
    }
    newBlock->next=nextBlock;
    merge(newBlock, nextBlock);
    merge(prevBlock, newBlock);
    return 0;
}

void MemoryAllocator::merge(BlockHeader *block1, BlockHeader *block2) {
    if(!block1 || !block2)
        return;
    //dva bloka ce se spojiti u jedan, dobijeni blok ce da krece od iste adrese kao prvi blok (nizi u memoriji)
    //ali ce sada njegova velicina biti kao ukupna velicina:

    //provera da li su blokovi susedni
    if((char*)block1+HEADER_SIZE+block1->size == (char*)block2){ //ako su blokovi susedni, spajaju se
        block1->next=block2->next;
        block1->size+=HEADER_SIZE+block2->size;
    }
}

