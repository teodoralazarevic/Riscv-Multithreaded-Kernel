//
// Created by os on 5/3/24.
//

#ifndef PROJECT_BASE_SYSCALL_C_HPP
#define PROJECT_BASE_SYSCALL_C_HPP
#include "../lib/hw.h"
#include "../h/tcb.hpp"

enum Code{
    MEM_ALLOC=0x01, MEM_FREE=0x02, THREAD_CREATE=0x11, THREAD_EXIT=0x12, THREAD_DISPATCH=0x13,
    SEM_OPEN=0x21, SEM_CLOSE=0x22, SEM_WAIT=0x23, SEM_SIGNAL=0x24, SEM_TIMEDWAIT=0x25,
    SEM_TRYWAIT=0x26, TIME_SLEEP=0x31, GETC=0x41, PUTC=0x42, JOIN=0x43, JOIN_TIME=0x44,
    JOIN_ALL=0x45, JOIN_ALL_TIMED=0x46, SET_PID=0x47, WAIT=0x48, SEND=0x49, RECEIVE=0x50,
    GET_THREAD_ID=0x56, ADD_OWNER=0x57, REMOVE_OWNER=0x58, GET_THREAD_ID_DISP=0x59,
    SET_MAX_THREADS=0x60
};

void* mem_alloc(size_t size);
int mem_free(void*);

class TCB;
typedef TCB* thread_t;
int thread_create(thread_t* handle, void(*start_routine)(void*), void* arg);
int thread_exit();
void thread_dispatch();
void thread_join(thread_t handle);
void thread_timed_join(thread_t handle, time_t time);
void join_all();
void join_all_timed(time_t time);
void set_pid(thread_t myHandle, uint64 pid);
void _wait(uint64 pid);
void _send(thread_t myHandle, const char* message);
char* _receive();
uint64 get_thread_id();
uint64 get_thread_id_disp();
void set_max_threads(uint64 num);

class Sem;
typedef Sem* sem_t;
int sem_open(sem_t* handle, uint32 init);
int sem_close(sem_t handle);
int sem_wait(sem_t id);
int sem_signal(sem_t id);
int sem_timedwait(sem_t id, time_t timeout);
int sem_trywait(sem_t id);
void add_owner(sem_t myHandle);
void remove_owner(sem_t myHandle, uint64 id);

int time_sleep(time_t);

const int EOF=-1;
char getc();
void putc(char);

#endif //PROJECT_BASE_SYSCALL_C_HPP
