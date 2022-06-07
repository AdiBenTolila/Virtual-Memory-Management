/**
   ex5:
    The program implements a simulation of processor access to memory and the program uses a paging mechanism that allows programs to be run only when even part of it is in the memory.
    We will divide the memory of the program into pages and if necessary load pages  These are for memory and we will use them (we will access them by struct and so we will know if the page is in memory, can we write / read from it
    if it is in swap where it is ,etc.)
    By functions:
    load - that access specific pages and if they are not in the processor  we can transfer this page to memory and then we can return the requested character (the method gets a logical address and accesses it for reading data
    and of course makes sure the relevant page of the requested process is in main memory)
    store- similar to load will load the desired page if it is not in the memory processor and write according to the appropriate place (the method also receives a logical address to be accessed for writing a data
    and also makes sure that the appropriate page is in memory)


   - Input Discounts:
    *Text pages are not allowed to write
    *if loading a heap_stack page for the first time-  it would be an error
    *When a store  to a page that is not found and is of type bss / heap_stack it will enter zeros to the memory
    * can be just one or two process
**/

#ifndef SIM_MEM
#define SIM_MEM

#include <iostream>
#include <string>
#include <cstdlib>
#include <queue>
using namespace std;
#define MEMORY_SIZE 30
extern char main_memory[MEMORY_SIZE];

typedef struct page_descriptor
{
    int V; // valid
    int D; // dirty
    int P; // permission
    int frame; //the number of a frame if in case it is page-mapped
    int swap_index; // where the page is located in the swap file.
} page_descriptor;

class sim_mem {
    //i change the type!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    int swapfile_fd; //swap file fd
    int program_fd[2]{}; //executable file fd
    int text_size;
    int data_size;
    int bss_size;
    int heap_stack_size;
    int num_of_pages;
    int page_size;
    int num_of_proc;
    page_descriptor **page_table; //pointer to page table.!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

public:
    sim_mem(char exe_file_name1[],char exe_file_name2[],char swap_file_name[], int text_size, int data_size, int bss_size,int heap_stack_size, int num_of_pages, int page_size, int num_of_process);
    ~sim_mem();
    char load(int, int);
    void store(int, int, char);
    void print_memory();
    void print_swap () const;
    void print_page_table();
private:
    bool *placeInRam;
    int countSwap = 0;
    std:: queue <page_descriptor *> queuePage;
    void putInMemory(int, int, int*);
    void findPlace(int*);
};

#endif