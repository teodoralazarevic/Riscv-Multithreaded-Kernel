//
// Created by os on 8/1/24.
//

#ifndef PROJECT_BASE_BUFFERC_HPP
#define PROJECT_BASE_BUFFERC_HPP
#include "../lib/hw.h"
#include "sem.hpp"

class Bufferc{
public:
    Bufferc();
    char getc();
    void putc(char ch);
    int getSize();
private:
    Sem* itemAvailable;
    Sem* spaceAvilable;
    static const uint64 capacity=64;
    char buff[capacity];
    uint64 head, tail;
    int count;
};
#endif //PROJECT_BASE_BUFFERC_HPP
