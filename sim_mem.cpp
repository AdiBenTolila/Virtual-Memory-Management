#include <string>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>

using namespace std;
#include "sim_mem.h"
#define MEMORY_SIZE 30
#define TRUE 1
#define FALSE 0
#define EMPTY TRUE//to know if there is an empty place


/**
 * constructor- initialize all the files (array memory,page table struct, array of the place in the memory).
  @param  exe_file_name1 - File name of the first process
  @param  exe_file_name2 - The file name of the second process (if it does not exist will enters quoted)
  @param  swap_file_name - SWAP file name (initialize the file with zeros)
  @param  text_size - size of the text (in bites)
  @param  data_size - size of the data (in bites)
  @param  bss_size - size of the bss (in bites)
  @param  heap_stack_size - size of the heap_stack (in bites)
  @param  num_of_pages - num of the page in the memory
  @param  page_size - size of the page(and frame) in the system
  @param  num_of_process - num of the process in the system (can be one or two)
 **/
sim_mem::sim_mem(char exe_file_name1[],char exe_file_name2[],char swap_file_name[], int text_size, int data_size, int bss_size, int heap_stack_size,int num_of_pages, int page_size, int num_of_process){

    this -> text_size = text_size;//cant read
    this -> data_size = data_size;
    this -> bss_size = bss_size;
    this -> heap_stack_size = heap_stack_size;
    this -> num_of_pages = num_of_pages;
    this -> page_size = page_size;
    this -> num_of_proc = num_of_process;

    this -> swapfile_fd = open(swap_file_name, O_RDWR|O_CREAT, 0666);//open or creat the swap file
    if(this -> swapfile_fd < 0){
        perror("fetch file failed");
        exit(1);
    }
    if(num_of_process != 0){
        this -> program_fd[0] = open(exe_file_name1, O_RDONLY);
        if(this -> program_fd[0] < 0){
            perror("fetch file failed");
            exit(1);
        }
    }
    if(num_of_process == 2){
        this -> program_fd[1] = open(exe_file_name2, O_RDONLY);
        if(this -> program_fd[1] < 0){
            perror("fetch file failed");
            exit(1);
        }
    }
    for (char & i : main_memory)//initialize the main memory
        i = '0';
    int numOfZero = 0;
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while( numOfZero < (page_size * (num_of_pages - (text_size/page_size)) )){
        write(swapfile_fd,"0", 1);
        numOfZero++;
    }
    page_table = (page_descriptor**)malloc(sizeof(page_descriptor*)*(this -> num_of_proc));
    for (int j = 0; j < num_of_proc; j++) {//initialize the page table
        page_table[j] = (page_descriptor*)malloc(sizeof(page_descriptor)*(this -> num_of_pages));
        if (page_table[j] == nullptr) {
            printf("no memory to allocate");
            this ->~sim_mem();
            exit(1);
        }
        for(int i = 0; i < (this -> num_of_pages); i++) {
                page_table[j][i].V = 0;//valid-1,invalid-0
                page_table[j][i].D = 0;//is dirty - 1 , not dirty-0
                page_table[j][i].frame = -1;//find empty place
                page_table[j][i].swap_index = -1;
                if( i < ( this -> text_size) / this -> page_size)//ask asaf!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                    page_table[j][i].P = 0;//if there is no permission put in the main memory zero permission - 1 ,not permission - 0
                else
                    page_table[j][i].P = 1;
        }
    }
    placeInRam = (bool*)malloc((MEMORY_SIZE / page_size) *sizeof(bool));
    if (placeInRam == nullptr) {
        printf("no memory to allocate");
        this ->~sim_mem();
        exit(1);
    }
    for (int i = 0; i < MEMORY_SIZE / page_size ; ++i) {
        placeInRam[i] = EMPTY;
    }
}
/**************************************************************************************/
sim_mem::~sim_mem (){
    for (int i = 0; i < num_of_proc; ++i) {//free struct
        if (page_table[i] != nullptr) {
            free(page_table[i]);
            page_table[i] = nullptr;
        }
    }
    //close files
    free(placeInRam);
    close(swapfile_fd);
    if(num_of_proc != 0)
        close(program_fd[0]);
    if(num_of_proc == 2)
        close(program_fd[1]);

}
/**************************************************************************************/
char sim_mem:: load(int process_id, int address){
    process_id--;
    int numPage = address / this -> page_size;
    int hist = address % this -> page_size;
    int placeInMemory = 0;
    char val;
    char str[page_size];
    str[0] = '\0';
    if(numPage > num_of_pages){
        fprintf(stderr,"no such address");
        this -> ~sim_mem();
        exit(1);
    }

    if(page_table[process_id][numPage].V == 1){//find in the main memory
        int frame_address = page_table[process_id][numPage].frame * page_size;
        placeInRam[frame_address] = FALSE;
        return  main_memory[frame_address + hist];
    }
    if(page_table[process_id][numPage].V == 0 && page_table[process_id][numPage].D == 0 && address > text_size + data_size + bss_size){
        fprintf(stderr,"load first time heap/stack");
        this -> ~sim_mem();
        exit(1);
    }
    putInMemory(process_id,address,&placeInMemory);
    /*update the page table*/
    val = main_memory[placeInMemory + hist];
    return val;
}
/**************************************************************************************/
void sim_mem:: store(int process_id, int address, char value){
    process_id--;
    int numPage = address / this -> page_size;
    int hist = address % this -> page_size;
    int placeInMemory = 0;
    if(numPage > num_of_pages){
        fprintf(stderr,"no such address");
        this -> ~sim_mem();
        exit(1);
    }
    if(page_table[process_id][numPage].P == 0){//not have a permission
        fprintf(stderr,"no permission");
        this -> ~sim_mem();
        exit(1);
    }
    if(page_table[process_id][numPage].V == 1){
        main_memory[(page_table[process_id][numPage].frame)*page_size + hist] = value;
        page_table[process_id][numPage].D = 1;
        return;
    }
    page_table[process_id][numPage].D = 1;
    putInMemory(process_id,address,&placeInMemory);

    if(address > text_size + data_size){
        for (int i = 0; i < page_size; ++i)
            main_memory[placeInMemory + i] = '0';
        page_table[process_id][numPage].V = 1;//now is valid
        page_table[process_id][numPage].frame = (placeInMemory) / page_size;
        placeInRam[(placeInMemory)/ page_size] = FALSE;
    }
    main_memory[placeInMemory + hist] = value;
}
/**************************************************************************************/
void sim_mem::print_memory() {
    int i;
    printf("\n Physical memory\n");
    for(i = 0; i < MEMORY_SIZE; i++) {
        if((i % this->page_size) == 0)
            printf("\n");
        printf("[%c]\t", main_memory[i]);
    }
    printf("\n");
}
/************************************************************************************/
void sim_mem::print_swap() const {
    char* str = (char*)malloc(this->page_size * sizeof(char));
    int i,numOfZero = 0;
    printf("\n Swap memory\n");
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while( numOfZero < (page_size*(num_of_pages - (text_size/page_size)) )) {
        read(swapfile_fd, str, this->page_size);
        for(i = 0; i < page_size; i++) {
            printf("%d - [%c]\t", i, str[i]);
        }
        printf("\n");
        numOfZero+=page_size;
    }
    free(str);
}
/***************************************************************************************/
void sim_mem::print_page_table() {
    int i;
    for (int j = 0; j < num_of_proc; j++) {
        printf("\n page table of process: %d \n", j);
        printf("Valid\t Dirty\t Permission \t Frame\t Swap index\n");
        for(i = 0; i < num_of_pages; i++) {
            printf("[%d]\t[%d]\t[%d]\t[%d]\t[%d]\n",
                   page_table[j][i].V,
                   page_table[j][i].D,
                   page_table[j][i].P,
                   page_table[j][i].frame ,
                   page_table[j][i].swap_index);
        }
    }
}
/***************************************************************************************/
void sim_mem:: findPlace(int* placeInMemory){
    char str[page_size];
    str[0] = '\0';
    int found = FALSE;
    page_descriptor* pge = queuePage.front();
    queuePage.pop();
    if(pge -> V == 1){
        (*placeInMemory) = pge -> frame * page_size;
        if(pge -> D == 1){ //dirty-put in swap

            if(countSwap >= (page_size * (num_of_pages - (text_size/page_size)) )){
                printf("no place in the swap file\n");
                this -> ~sim_mem();
                exit(1);
            }
            countSwap += page_size;
            found = FALSE;
            int count = 0;
            str[0] = '\0';//to know were to find an empty place
            lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
            while(!found) {//found empty place in the swap file in the first place
                found = TRUE;
                read(swapfile_fd, str, page_size);
                for (int i = 0; i < page_size; ++i) {
                    if (str[i] != '0')
                        found = FALSE;
                }
                count = count + page_size;
            }
            lseek(swapfile_fd, count - page_size, SEEK_SET);
            write(swapfile_fd, main_memory + (*placeInMemory), page_size);
            pge -> swap_index = count - page_size;//find in the swap and put what in the main memory
        }
        pge -> V = 0;
        pge -> frame = -1;
    }
}
/***************************************************************************************/
void sim_mem:: putInMemory(int process_id, int address,int* placeInMemory ){
    int numPage = address / this -> page_size;
    int hist = address % this -> page_size;
    //int found = FALSE;
    int j;
    if(queuePage.size() < (MEMORY_SIZE / page_size)){//queuePage.size()
        for (j = 0; !placeInRam[j] ; ++j);//find empty place
        (*placeInMemory) = j * page_size;
    }
    else{//find place in the memory
        findPlace(placeInMemory);
    }
    /*put in the main memory or error if it's in the heap stack*/
    if(page_table[process_id][numPage].swap_index != -1){//if it's in the swap move it from the swap to the main memory
        lseek(swapfile_fd, page_table[process_id][numPage].swap_index, SEEK_SET); // go to the start of the file
        read(swapfile_fd, main_memory + (*placeInMemory), page_size);//read from the swap to the main memory
        lseek(swapfile_fd, page_table[process_id][numPage].swap_index, SEEK_SET); // go to the start of the file
        for (int k = 0; k < page_size; ++k) {
            write(swapfile_fd, "0", 1);//delete from the swap file
        }
        page_table[process_id][numPage].swap_index = -1;
        countSwap -= page_size;
    }
    else{//if it's not in the swap or in the ram do according the case
        if(address < text_size + data_size){
            lseek(program_fd[process_id], address - hist, SEEK_SET); // go to the place of the page in the file
            read(program_fd[process_id], main_memory + (*placeInMemory), page_size);//find in the exec and copy to the memory
        }
        else {
            /*in the bss or not load in the first time*/
            if(page_table[process_id][numPage].D == 1 || (address <  ((text_size + data_size + bss_size)))){
                for (int i = 0; i < page_size; ++i)
                    main_memory[(*placeInMemory) + i] = '0';
            }
            else{//heap_stack - the first time that we load that
                return;
            }
        }
    }

    page_table[process_id][numPage].V = 1;//now is valid
    page_table[process_id][numPage].frame = (*placeInMemory) / page_size;
    placeInRam[(*placeInMemory)/ page_size] = FALSE;
    queuePage.push(&(page_table[process_id][numPage]));
}
/***************************************************************************************/
