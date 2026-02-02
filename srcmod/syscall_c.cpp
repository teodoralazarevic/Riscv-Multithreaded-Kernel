//
// Created by os on 5/3/24.
//
#include "../h/syscall_c.hpp"
#include "../h/memoryAllocator.hpp"
#include "../lib/console.h"

void syscall(uint64 code, uint64 arg1=0, uint64 arg2=0, uint64 arg3=0, uint64 arg4=0){
    __asm__ volatile("ecall");
}

void* mem_alloc(size_t size){
    if(!size) return nullptr;
    //vrsimo zaokruzivanje na blokove pre poziva ABI-a
    size_t newSize= size + sizeof(BlockHeader);
    newSize=newSize%MEM_BLOCK_SIZE?(newSize/MEM_BLOCK_SIZE+1):newSize/MEM_BLOCK_SIZE;
    syscall(MEM_ALLOC, newSize);
    void volatile* ret;
    __asm__ volatile("mv %0, a0" : "=r"(ret));
    return (void*)ret;
}

int mem_free(void* pointer){
    syscall(MEM_FREE, (uint64) pointer);
    int volatile ret;
    __asm__ volatile("mv %0, a0" : "=r"(ret));
    return ret;
}

int thread_create(thread_t* handle, void(*start_routine)(void*), void* arg){
    void* volatile stack= nullptr;
    if(start_routine!= nullptr)
        stack= mem_alloc(DEFAULT_STACK_SIZE*sizeof(size_t));
    syscall(THREAD_CREATE, (uint64)handle, (uint64) start_routine, (uint64) arg, (uint64) stack);
    int volatile ret;
    __asm__ volatile("mv %0, a0" : "=r"(ret));
    return ret;
}

int thread_exit(){
    syscall(THREAD_EXIT);
    int volatile ret;
    __asm__ volatile("mv %0, a0" : "=r"(ret));
    return ret;
}

void thread_join(thread_t handle){
    syscall(JOIN, (uint64) handle);
}

void thread_timed_join(thread_t handle, time_t time){
    syscall(JOIN_TIME, (uint64)handle, (uint64)time);
}

void join_all(){
    syscall(JOIN_ALL);
}

void join_all_timed(time_t time){
    syscall(JOIN_ALL_TIMED, (uint64)time);
}

void set_pid(thread_t myHandle, uint64 pid){
    syscall(SET_PID, (uint64)myHandle, (uint64)pid);
}

void _wait(uint64 pid){
    syscall(WAIT, (uint64)pid);
}

void _send(thread_t myHandle, const char* message){
    syscall(SEND, (uint64)myHandle, (uint64)message);
}

uint64 get_thread_id(){
    syscall(GET_THREAD_ID);
    uint64 volatile ret;
    __asm__ volatile("mv %0, a0" : "=r"(ret));
    return ret;
}

void add_owner(sem_t myHandle){
    syscall(ADD_OWNER, (uint64)myHandle);
}

void remove_owner(sem_t myHandle, uint64 id){
    syscall(REMOVE_OWNER, (uint64)myHandle, (uint64)id);
}

uint64 get_thread_id_disp(){
    syscall(GET_THREAD_ID_DISP);
    uint64 volatile ret;
    __asm__ volatile("mv %0, a0" : "=r"(ret));
    return ret;
}

void set_max_threads(uint64 num){
    syscall(SET_MAX_THREADS, (uint64)num);
}

char* _receive(){
    syscall(RECEIVE);
    char* volatile ret;
    __asm__ volatile("mv %0, a0" : "=r"(ret));
    return ret;
}

void thread_dispatch(){
    syscall(THREAD_DISPATCH);
}

int sem_open(sem_t* handle, uint32 init){
    syscall(SEM_OPEN, (uint64)handle, (uint64)init);
    int volatile ret;
    __asm__ volatile("mv %0, a0" : "=r"(ret));
    return ret;
}

int sem_close(sem_t handle){
    syscall(SEM_CLOSE, (uint64) handle);
    int volatile ret;
    __asm__ volatile("mv %0, a0" : "=r"(ret));
    return ret;
}

int sem_wait(sem_t id){
    syscall(SEM_WAIT, (uint64) id);
    int volatile ret;
    __asm__ volatile("mv %0, a0" : "=r"(ret));
    return ret;
}
int sem_signal(sem_t id){
    syscall(SEM_SIGNAL, (uint64) id);
    int volatile ret;
    __asm__ volatile("mv %0, a0" : "=r"(ret));
    return ret;
}

char getc(){
    char ch='A';
    char* ret= &ch;
    syscall(GETC, (uint64)ret);
    return *ret;
}

void putc(char ch){
    syscall(PUTC, (uint64)ch);
}

int sem_timedwait(sem_t id, time_t timeout){
    syscall(SEM_TIMEDWAIT, (uint64) id, (uint64)timeout);
    int volatile ret;
    __asm__ volatile("mv %0, a0" : "=r"(ret));
    return ret;
}

int sem_trywait(sem_t id){
    syscall(SEM_TRYWAIT, (uint64) id);
    int volatile ret;
    __asm__ volatile("mv %0, a0" : "=r"(ret));
    return ret;
}

int time_sleep(time_t time){
    syscall(TIME_SLEEP, (uint64) time);
    int volatile ret;
    __asm__ volatile("mv %0, a0" : "=r"(ret));
    return 0;
}