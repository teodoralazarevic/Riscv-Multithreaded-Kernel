# Riscv-Multithreaded-Kernel
## Overview
The goal of this project is to implement a small but functional operating system kernel for the RISC-V processor. The kernel supports:

- Multithreading with time sharing
- Semaphores for synchronization
- Asynchronous context switching triggered by timer and keyboard interrupts
- Memory allocation
- Console input/output
 
> **Academic context:**  
> This project was developed as part of an **academic course on Operating Systems 1**, with the goal of gaining hands-on experience with core operating system concepts, low-level system programming, and kernel–user space interaction.


---

## System Architecture

### Interface Layers

- **ABI (Application Binary Interface)**  
  Binary interface for system calls implemented via software interrupts.

- **C API**  
  Procedural interface for system calls.

- **C++ API**  
  Object-oriented interface for system calls.

---

## Hardware Platform

- **Processor:** RISC-V (RV64IMA variant)
- **Emulator:** QEMU
- **Host System:** Modified xv6

---

## Project Structure

### Main Components

- **MemoryAllocator**  
  Memory allocator with contiguous allocation.

- **Thread / PCB**  
  Thread and context management.

- **Scheduler**  
  Scheduling algorithm (e.g. FIFO).

- **Semaphore**  
  Semaphore implementation.

- **Console**  
  Console management (UART).

---

## System Calls

### Memory Management

- `mem_alloc(size_t size)` – memory allocation  
- `mem_free(void*)` – memory deallocation

### Thread Management

- `thread_create()` – thread creation  
- `thread_exit()` – thread termination  
- `thread_dispatch()` – synchronous context switch  
- `time_sleep()` – thread sleep

### Semaphores

- `sem_open()` – semaphore creation  
- `sem_close()` – semaphore destruction  
- `sem_wait()` – wait operation  
- `sem_signal()` – signal operation  
- `sem_timedwait()` – timed wait  
- `sem_trywait()` – non-blocking wait

### Console

- `getc()` – read a character  
- `putc()` – write a character

---

## Code Organization

### Directories

- `srcmod/` – source files (`.cpp` and `.S`)
- `hmod/` – header files (`.h` and `.hpp`)


