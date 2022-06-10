#include "sim_mem.h"
using namespace std;

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

    this -> text_size = text_size;//can't write
    this -> data_size = data_size;
    this -> bss_size = bss_size;
    this -> heap_stack_size = heap_stack_size;
    this -> num_of_pages = num_of_pages;
    this -> page_size = page_size;
    this -> num_of_proc = num_of_process;
    program_fd[0] = -1;
    program_fd[1] = -1;

    if(strcmp(exe_file_name1 , "") != 0){
        this -> program_fd[0] = open(exe_file_name1, O_RDONLY);//open to read
        if(this -> program_fd[0] < 0){
            perror("fetch file failed");
            exit(1);
        }
    }
    if(strcmp(exe_file_name2 , "") != 0){
        this -> program_fd[1] = open(exe_file_name2, O_RDONLY);//open to read
        if(this -> program_fd[1] < 0){
            perror("fetch file failed");
            if(program_fd[0] != -1)
                close(program_fd[0]);
            exit(1);
        }
    }
    for (char & i : main_memory)//initialize the main memory
        i = '0';

    this -> swapfile_fd = open(swap_file_name, O_CREAT|O_RDWR,S_IRUSR|S_IWUSR);//open or creat the swap file
    if(this -> swapfile_fd < 0){
        perror("fetch file failed");
        if(program_fd[0] != -1)
            close(program_fd[0]);
        if(program_fd[1] != -1)
            close(program_fd[1]);
        exit(1);
    }
    int numOfZero = 0;
    sizeSwap = (num_of_proc * page_size * (num_of_pages - (text_size / page_size)) );//initialize the swap file
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while(numOfZero < sizeSwap){
        if(write(swapfile_fd,"0", 1) < 1){
            perror("failed to write");
            close(swapfile_fd);
            if(program_fd[0] != -1)
                close(program_fd[0]);
            if(program_fd[1] != -1)
                close(program_fd[1]);
            exit(1);
        }
        numOfZero++;
    }
    page_table = (page_descriptor**)malloc(sizeof(page_descriptor*)*(this -> num_of_proc));
    if (page_table == nullptr) {
        perror("no memory to allocate");
        close(swapfile_fd);
        if(program_fd[0] != -1)
            close(program_fd[0]);
        if(program_fd[1] != -1)
            close(program_fd[1]);
        exit(1);
    }

    for (int j = 0; j < num_of_proc; j++) {//initialize the page table
        page_table[j] = (page_descriptor*)malloc(sizeof(page_descriptor)*(this -> num_of_pages));
        if (page_table[j] == nullptr) {
            perror("no memory to allocate");
            close(swapfile_fd);
            if(program_fd[0] != -1)
                close(program_fd[0]);
            if(program_fd[1] != -1)
                close(program_fd[1]);
            if(j!=0)
                free(page_table[1]);
            free(page_table);
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
    placeInRam = (bool*)malloc((MEMORY_SIZE / page_size) *sizeof(bool));//to know where there is an empty place in the ram
    if (placeInRam == nullptr) {
        perror("no memory to allocate");
        close(swapfile_fd);
        if(program_fd[0] != -1)
            close(program_fd[0]);
        if(program_fd[1] != -1)
            close(program_fd[1]);
        for (int j = 0; j < num_of_proc; j++)
                free(page_table[1]);
        free(page_table);
        exit(1);
    }
    for (int i = 0; i < MEMORY_SIZE / page_size ; ++i) {
        placeInRam[i] = EMPTY;
    }

    emptyPlaceInSwap = (bool*)malloc((sizeSwap / page_size) * sizeof(bool));//to know where there is an empty place in the swap
    if (emptyPlaceInSwap == nullptr) {
        perror("no memory to allocate");
        close(swapfile_fd);
        if(program_fd[0] != -1)
            close(program_fd[0]);
        if(program_fd[1] != -1)
            close(program_fd[1]);
        for (int j = 0; j < num_of_proc; j++)
            free(page_table[1]);
        free(page_table);
        free(placeInRam);
        exit(1);
    }
    for (int i = 0; i < sizeSwap / page_size ; ++i) {
        emptyPlaceInSwap[i] = EMPTY;
    }
}
/**
 * destructor- free all teh allocates and close the files.
 **/
sim_mem::~sim_mem (){
    if(page_table != nullptr){
    for (int i = 0; i < num_of_proc; ++i) {//free struct
        if (page_table[i] != nullptr) {
            free(page_table[i]);
            page_table[i] = nullptr;
        }
    }

    //free allocates
    free(page_table);
    }
    if(placeInRam != nullptr){
        free(placeInRam);
    }
    if(emptyPlaceInSwap != nullptr){
        free(emptyPlaceInSwap);
    }
    //close files
    close(swapfile_fd);
    if(num_of_proc != 0)
        close(program_fd[0]);
    if(num_of_proc == 2)
        close(program_fd[1]);

}
/**
 * allowing access specific pages and if they are not in the processor  we can transfer this page to memory and then we can return the requested character (the method gets a logical address and accesses it for reading data
   and of course makes sure the relevant page of the requested process is in main memory)
 - if loading in the first time page type heap_stack - do nothing and the method output a message about that
  @param  process_id - num of process
  @param  address - logical address
 **/
char sim_mem:: load(int process_id, int address){
    process_id--;
    int numPage = address / this -> page_size;
    int offset = address % this -> page_size;
    int placeInMemory = 0;
    char val;
    char str[page_size];
    str[0] = '\0';
    if(numPage < 0 || numPage > num_of_pages){//The address is not in the field
        fprintf(stderr,"no such address");
        return '\0';
    }
    /*loading in the first time page type heap_stack*/
    if(page_table[process_id][numPage].V == 0 && page_table[process_id][numPage].D == 0 && address > text_size + data_size + bss_size){
        fprintf(stderr,"first time loading heap/stack");
        return '\0';
    }
    if(page_table[process_id][numPage].V == 1){//in the main memory
        int frame_address = page_table[process_id][numPage].frame * page_size;//get the place in the Ram
        placeInRam[page_table[process_id][numPage].frame] = FALSE;//update that this place in the Ram is occupied
        return  main_memory[frame_address + offset];//return the requested char
    }

    putInMemory(process_id,address,&placeInMemory);//method that if the requested page not in the Ram find place and insert to the ram the page

    val = main_memory[placeInMemory + offset];
    return val;//return the requested char
}
/**
 * The method will load the desired page if it is not in the memory processor and write according to the appropriate place (the method also receives a logical address to be accessed for writing a data
   and also makes sure that the appropriate page is in memory)
 - if storing page type text - do nothing and the method output a message about that
  @param  process_id - num of process
  @param  address - logical address
  @param  value - value to update the page
 **/
void sim_mem:: store(int process_id, int address, char value){
    process_id--;
    int numPage = address / this -> page_size;
    int offset = address % this -> page_size;
    int placeInMemory = 0;
    if(numPage < 0 || numPage > num_of_pages){//The address is not in the field
        fprintf(stderr,"no such address");
        return;
    }
    if(page_table[process_id][numPage].P == 0){//not have a permission to write
        fprintf(stderr,"no permission to write");
        return;
    }
    if(page_table[process_id][numPage].V == 1){//in the main memory
        main_memory[(page_table[process_id][numPage].frame)*page_size + offset] = value;//put in the right place in the ram tha value
        page_table[process_id][numPage].D = 1;
        return;
    }
    page_table[process_id][numPage].D = 1;
    putInMemory(process_id,address,&placeInMemory);//the request page not in the ram so "putInMemory" taking care of that

    if(address > text_size + data_size){//if its bss or heap_stack type it's not in the program file - need to insert zeros instead
        for (int i = 0; i < page_size; ++i)
            main_memory[placeInMemory + i] = '0';
        page_table[process_id][numPage].V = 1;//now is valid
        page_table[process_id][numPage].frame = (placeInMemory) / page_size;
        placeInRam[(placeInMemory) / page_size] = FULL;
    }
    main_memory[placeInMemory + offset] = value; //put in the right place in the ram tha value
}
/**
 * The method will bring place from the memory through fifo(queue)
 * the method move the page (of the place that found) to the swap memory or override it(if its not dirty)
  @param  placeInMemory - place in the memory to put the request page
 **/
void sim_mem:: findPlace(int* placeInMemory){
    char str[page_size];
    str[0] = '\0';
    page_descriptor* pge = queuePage.front();//take the first var that insert to the queue
    if(!queuePage.empty()){
        queuePage.pop();//delete var from the queue because it's not in the ram anymore
    }
    if(pge -> V == 1){
        (*placeInMemory) = pge -> frame * page_size;//the place that we want to the request page
        if(pge -> D == 1){ //dirty-put in swap

            if(countSwap > sizeSwap){
                printf("no place in the swap file\n");
                return;
            }
            countSwap += page_size;
            for (int i = 0; i < sizeSwap / page_size ; ++i) {
                if(emptyPlaceInSwap[i] == EMPTY){//found empty place in the swap and put the page there(because its dirty, and we don't want to lose it)
                    lseek(swapfile_fd, i * page_size, SEEK_SET); // point to the empty place in file
                    if(write(swapfile_fd, main_memory + (*placeInMemory), page_size) < 0){//write the page from the memory to the swap
                        fprintf(stderr,"failed to write");
                        return;
                    }
                    pge -> swap_index = i * page_size ;
                    emptyPlaceInSwap[i] = FULL;
                    break;
                }
           }
        }
        pge -> V = 0;
        pge -> frame = -1;
    }
}
/**
  Put the request page in the memory
  @param  process_id - num of process
  @param  address - logical address
  @param  placeInMemory - place in the memory to put the request page
 **/
void sim_mem:: putInMemory(int process_id, int address,int* placeInMemory ){
    int numPage = address / this -> page_size;
    int offset = address % this -> page_size;
    int j;
    if(queuePage.size() < (MEMORY_SIZE / page_size)){//if the ram not full
        for (j = 0; !placeInRam[j] ; ++j);//find empty place
        (*placeInMemory) = j * page_size;//the place that we want to the request page
    }
    else{//find place in the memory - FIFO
        findPlace(placeInMemory);
    }
    /*put in the main memory*/
    if(page_table[process_id][numPage].swap_index != -1){//if it's in the swap move it from the swap to the main memory
        transferFromSwapToRam(process_id,numPage,placeInMemory);
    }
    else{//if it's not in the swap or in the ram do according the case
        if(address < text_size + data_size){
            lseek(program_fd[process_id], address - offset, SEEK_SET); // go to the place of the page in the file
            if(read(program_fd[process_id], main_memory + (*placeInMemory), page_size) < 0){
                perror("failed to read");
                return;
            }
        }
        else {
            /*in the bss or not load in the first time*/
                if(page_table[process_id][numPage].D == 1 || (address <  ((text_size + data_size + bss_size)))){
                    for (int i = 0; i < page_size; ++i)
                        main_memory[(*placeInMemory) + i] = '0';
                }
                else{//heap_stack - the first time that we load that(never get to here)
                    return;
                }
        }
    }

    page_table[process_id][numPage].V = 1;//now is valid
    page_table[process_id][numPage].frame = (*placeInMemory) / page_size;
    placeInRam[(*placeInMemory)/ page_size] = FALSE;
    queuePage.push((&page_table[process_id][numPage]));//put in the queue
}
/**
  transfer the page from the swap to the main memory
  @param  process_id - num of process
  @param  numPage - num of the request page
  @param  placeInMemory - place in the memory to put the request page
 **/
void sim_mem:: transferFromSwapToRam(int process_id, int numPage,const int* placeInMemory ){
    lseek(swapfile_fd, page_table[process_id][numPage].swap_index, SEEK_SET); //go to the page swap index of the file
    if(read(swapfile_fd, main_memory + (*placeInMemory), page_size) < 0){
        perror("failed to read");
        return;
    }
    lseek(swapfile_fd, page_table[process_id][numPage].swap_index, SEEK_SET); // go to the page swap index of the file to initialize it(with zero)
    for (int k = 0; k < page_size; ++k) {
        if(write(swapfile_fd, "0", 1)<0){//delete from the swap file
            perror("failed to write");
            return;
        }
    }
    emptyPlaceInSwap[page_table[process_id][numPage].swap_index / page_size ] = EMPTY;
    page_table[process_id][numPage].swap_index = -1;
    countSwap -= page_size;
}
/**
 *  print what in the RAM
 **/
void sim_mem::print_memory() const {
    int i;
    printf("\n Physical memory\n");
    for(i = 0; i < MEMORY_SIZE; i++) {
        if((i % this->page_size) == 0)
            printf("\n");
        printf("[%c]\t", main_memory[i]);
    }
    printf("\n");
}
/**
 *  print what in the swap file
 **/
void sim_mem::print_swap() const  {
    char* str = (char*)malloc(this->page_size *sizeof(char));
    if (str == nullptr) {
        perror("no memory to allocate");
        exit(1);
    }
    int i;
    printf("\n Swap memory\n");
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while(read(swapfile_fd, str, this->page_size) == this->page_size) {
        for(i = 0; i < page_size; i++) {
            printf("%d - [%c]\t", i, str[i]);
        }
        printf("\n");
    }
    free(str);
}
/**
 *  print what in the page table
 **/
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
