//
// Created by os on 5/3/24.
//
#include "../h/syscall_cpp.hpp"
#include "../lib/console.h"
void* operator new(size_t size){
    return mem_alloc(size);
}

void* operator new[](size_t size){
    return mem_alloc(size);
}

void operator delete(void* pointer){
    mem_free(pointer);
}

void operator delete[](void* pointer){
    mem_free(pointer);
}

Thread::Thread(void (*body)(void *), void *arg) {
    this->body=body;
    this->arg=arg;
}

Thread::~Thread() {
    while(!myHandle->isFinished())
        thread_dispatch();
    delete myHandle;
}

int Thread::start() {
    return thread_create(&myHandle, &threadWrapper, this);
}

void Thread::dispatch() {
    thread_dispatch();
}

Thread::Thread() {
    this->body= nullptr;
    this->arg= nullptr;
}

void Thread::threadWrapper(void* arg) {
    Thread* thr=(Thread*) arg;
    if(thr->body)
        thr->body(thr->arg);
    else
        thr->run();
}

int Thread::sleep(time_t time){
    return time_sleep(time);
}

void Thread::join() {
    thread_join(myHandle);
}

void Thread::timedJoin(time_t time){
    thread_timed_join(myHandle, time);
}

void Thread::joinAll() {
    join_all();
}

void Thread::joinAllTimed(time_t time) {
    join_all_timed(time);
}

void Thread::setPID(uint64 pid) {
    set_pid(myHandle, pid);
}

void Thread::wait(uint64 pid) {
    _wait(pid);
}

void Thread::send(const char *message) {
    _send(myHandle, message);
}

char *Thread::receive() {
    return _receive();
}

uint64 Thread::getThreadID() {
    return get_thread_id();
}

uint64 Thread::getThreadIDDisp() {
    return get_thread_id_disp();
}

void Thread::setMaxThreads(uint64 num) {
    set_max_threads(num);
}


Semaphore::Semaphore(uint32 init) {
    sem_open(&myHandle, init);
}

Semaphore::~Semaphore() {
    sem_close(myHandle);
}

int Semaphore::wait() {
    return sem_wait(myHandle);
}

int Semaphore::signal() {
    return sem_signal(myHandle);
}

int Semaphore::timedWait(time_t timeout) {
    return sem_timedwait(myHandle, timeout);
}

int Semaphore::tryWait() {
    return sem_trywait(myHandle);
}

void Semaphore::addOwner() {
    add_owner(myHandle);
}

void Semaphore::removeOwner(uint64 id) {
    remove_owner(myHandle, id);
}

char Console::getc() {
    return ::getc();
}

void Console::putc(char ch) {
    ::putc(ch);
}

PeriodicThread::PeriodicThread(time_t period){
    this->period=period;
}

void PeriodicThread::terminate(){
    this->period=0;
}

void PeriodicThread::run() {
    while(period){
        Thread::sleep(period);
        periodicActivation();
    }
}