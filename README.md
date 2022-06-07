# Virtual-Memory-Management
The program implements a simulation of processor access to memory and the program uses a paging mechanism that allows programs to be run only when even part of it is in the memory.
We will divide the memory of the program into pages and if necessary load pages  These are for memory and we will use them (we will access them by struct and so we will know if the page is in memory, can we write / read from it 
if it is in swap where it is ,etc.)
    By functions:
    load - that access specific pages and if they are not in the processor  we can transfer this page to memory and then we can return the requested character (the method gets a logical address and accesses it for reading data 
    and of course makes sure the relevant page of the requested process is in main memory)
    store- similar to load will load the desired page if it is not in the memory processor and write according to the appropriate place (the method also receives a logical address to be accessed for writing a data   
    and also makes sure that the appropriate page is in memory)

## Input Discounts:
* Text pages are not allowed to write
* if loading a heap_stack page for the first time-  it would be an error 
* When a store  to a page that is not found and is of type bss / heap_stack it will enter zeros to the memory
