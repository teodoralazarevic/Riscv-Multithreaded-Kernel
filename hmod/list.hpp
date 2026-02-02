//
// Created by os on 5/10/24.
//

#ifndef PROJECT_BASE_LIST_HPP
#define PROJECT_BASE_LIST_HPP
#include "memoryAllocator.hpp"
template<typename T>
class List{
private:
    struct Node {
        T* data;
        Node *next;

        Node(T* data, Node* next) : data(data), next(next) {}

        void* operator new(size_t size){
            size_t newSize= size + sizeof(BlockHeader);
            newSize=newSize%MEM_BLOCK_SIZE?(newSize/MEM_BLOCK_SIZE+1):newSize/MEM_BLOCK_SIZE;
            void* volatile adr=MemoryAllocator::memAlloc(newSize);
            return adr;
        }

        void* operator new[](size_t size){
            size_t newSize= size + sizeof(BlockHeader);
            newSize=newSize%MEM_BLOCK_SIZE?(newSize/MEM_BLOCK_SIZE+1):newSize/MEM_BLOCK_SIZE;
            void* volatile adr=MemoryAllocator::memAlloc(newSize);
            return adr;
        }

        void operator delete(void* pointer){
            MemoryAllocator::memFree(pointer);
        }

        void operator delete[](void* pointer){
            MemoryAllocator::memFree(pointer);
        }
    };

    Node *head, *tail;
public:
    List() : head(0), tail(0) {}
    //List(Node* head, Node* tail):head(head), tail(tail){}

    List(const List<T> &) = delete;
    List<T> &operator=(const List<T> &) = delete;

    void addFirst(T* data){
        Node* node=new Node(data, head);
        head=node;
        if(!tail) tail=head;
    }

    void addLast(T* data){
        Node* node=new Node(data, 0);
        if(tail) {
            tail->next = node;
            tail=node;
        }
        else
            head=tail=node;
    }

    T* removeFirst(){
        if(!head) return 0;
        Node* elem=head;
        head=head->next;
        if(!head) tail= 0;
        T* data=elem->data;
        delete elem;
        return data;
    }

    T* removeLast(){
        if(!tail) return nullptr;
        Node* prev=0;
        for(Node* curr=head;curr && curr!=tail;curr=curr->next)
            prev=curr;
        Node* tmp=tail;
        if(prev) prev->next=0;
        else head=0;
        tail=prev;
        T* data=tmp->data;
        delete tmp;
        return data;
    }

    void remove(T* data){
        if(!head) return;
        Node* tmp=head, *prev= nullptr;
        while(tmp && *(tmp->data)!=*data){
            prev=tmp;
            tmp=tmp->next;
        }
        if(!tmp) return;
        if(!prev)
            head=tmp->next;
        else
            prev->next=tmp->next;

        if(!head)
            tail= nullptr;
        delete tmp;
    }

    bool elemInList(T* data){
        for(Node* curr=head;curr;curr=curr->next){
            if(*curr->data==*data)
                return true;
        }
        return false;
    }

    T* peekFirst(){
        if(!head)
            return nullptr;
        return head->data;
    }


    /*~List(){
        while(head){
            Node* tmp=head->next;
            delete head;
            head=tmp;
        }
    }*/
};

#endif //PROJECT_BASE_LIST_HPP
