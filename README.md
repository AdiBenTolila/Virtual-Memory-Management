# Virtual-Memory-Management

Authored by Adi Bentolila

## Description
    The program implements a simulation of processor access to memory and the program uses a paging mechanism that allows programs to be run only when even part of     it is in the memory.
    We will divide the memory of the program into pages and if necessary load pages  These are for memory and we will use them (we will access them by struct and so     we will know if the page is in memory, can we write / read from it 
    if it is in swap where it is ,etc.)
    By functions:
    load - that access specific pages and if they are not in the processor  we can transfer this page to memory and then we can return the requested character (the     method gets a logical address and accesses it for reading data 
    and of course makes sure the relevant page of the requested process is in main memory)
    store- similar to load will load the desired page if it is not in the memory processor and write according to the appropriate place (the method also receives a     logical address to be accessed for writing a data   
    and also makes sure that the appropriate page is in memory)


   ## Input Discounts:
    * Text pages are not allowed to write
    * if loading a heap_stack page for the first time-  it would be an error 
    * When a store  to a page that is not found and is of type bss / heap_stack it will enter zeros to the memory


## functions:
public:

    * sim_mem -    constructor- initialize all the files (array memory,page table struct, array of the place in the memory)
    
    * ~sim_mem -   destructor- free all teh allocates and close the files.
    
    * char load -  allowing access specific pages and if they are not in the processor  we can transfer this page to memory and then we can return the requested                        character (the method gets a logical address and accesses it for reading data
                  and of course makes sure the relevant page of the requested process is in main memory)
                  - if loading in the first time page type heap_stack - do nothing and the method output a message about that
    
    * void store - The method will load the desired page if it is not in the memory processor and write according to the appropriate place (the method also receives       a logical address to be accessed for writing a data and also makes sure that the appropriate page is in memory)
    
    * void print_memory - print what in the RAM.
    
    * void print_swap - print what in the swap file.
    
    *void print_page_table -  print what in the page table.

private:

    * void putInMemory - the method will bring place from the memory through fifo(queue), the method move the page (of the place that found) to the swap memory or         override it(if its not dirty).
    
    * void findPlace - put the request page in the memory
    
    * transferFromSwapToRam - transfer the page from the swap to the main memory

## Program Files

sim_mem.h - header: all the declaration

sim_mem.cpp - contain the program(the virtual memory management).

main.cpp - run the program

## How to compile?

```[bash]
compile: g++ main.cpp sim_mem.cpp -o main 
```
run:

```[bash]
./main
```

## Output:
- swapfile_fd - describes a file of the replacement file, to which we will refer pages from memory

- the output of what in the main
