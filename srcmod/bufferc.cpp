//
// Created by os on 8/1/24.
//
#include "../h/bufferc.hpp"

Bufferc::Bufferc() {
    head=tail=count=0;
    sem_open(&itemAvailable, 0);
    sem_open(&spaceAvilable, capacity);
}

char Bufferc::getc() {
    Sem::semWait(itemAvailable);
    char ch=buff[head];
    head=(head+1)%capacity;
    --count;
    Sem::semSignal(spaceAvilable);
    return ch;
}

void Bufferc::putc(char ch) {
    Sem::semWait(spaceAvilable);
    buff[tail] = ch;
    tail = (tail + 1) % capacity;
    ++count;
    Sem::semSignal(itemAvailable);
}

int Bufferc::getSize() {
    return count;
}

