#include "sysCalls.h"
#include "x86_desc.h"
#include "lib.h"
#include "filesys.h"
#include "scheduler.h"

#define START_OF_USER   0x800000
#define START_OF_KERNEL 0x400000
#define KERNEL_STACK_SIZE 0x2000
#define END_OF_VIRTUAL 0x08400000
#define EXEC_BYTE_1 0x7F
#define EXEC_BYTE_2 0x45
#define EXEC_BYTE_3 0x4C
#define EXEC_BYTE_4 0x46
#define PROG_IMAGE 0x08048000
#define PCB_SIZE 0x1000
#define PROG_IMAGE_OFFSET 0x8048018
#define ARG_MAX 128
#define FD_MIN 1
#define FD_MAX 7
#define FD_START 2
#define FD_END 8
#define STDIN 0
#define STDOUT 1
#define _128MB 0x8000000
#define _132MB 0x8400000
#define _136MB 0x8800000

uint32_t prog_eip;
uint8_t buf[4];
int tasks[6] = {0, 0, 0, 0, 0, 0};
int terminal_arr[3] = {-1, -1, -1};
int task_avail;
uint32_t eflag_bitmask = 0x200; //masks everything other than the IF bit
uint32_t stack_pointer = END_OF_VIRTUAL - sizeof(int32_t);
char buffer[64];    //buffer to transfer command into
extern void flush_tlb();
extern void goto_user(uint32_t SS, uint32_t ESP, uint32_t eflag_bitmask, uint32_t CS, uint32_t EIP);
uint32_t dir_index = 0;
int prog_counter = 0;
int pid = 0;
int bad_call();
extern void ret_to_exec(uint32_t old_ebp, uint32_t old_esp, uint32_t status);
void set_up_vidmap();

/* void map_memory(int pid)
 * Inputs: int pid - program id number of the task currently being handled
 * Return Value: none
 * Function: helper function that maps virtual memory to physical and flushes the TLB */
void map_memory(int pid){
    pde[32].page_table_base_addr = (((uint32_t) START_OF_USER) + (pid * START_OF_KERNEL)) >> 12;    //shifted 12 to make 20 bits long
    pde[32].user_supervisor = 1;
    pde[32].present = 1;
    pde[32].page_size = 1;
    flush_tlb(); //flushing the TLB
}

/* int check_executable(char* filename)
 * Inputs: char* filename - string holding the file name to be checked
 * Return Value: 0 if not an executable file, 1 otherwise
 * Function: helper function that checks if the given file is an executable file */
int check_executable(char* filename){
    dentry_t dentry;
    if(open_f((const uint8_t*)filename) == -1){ //returns 0 if file can't be opened
		return 0;
	}
    read_dentry_by_name((const uint8_t*)filename, &dentry); //get the directory entry of the given file
    read_data(dentry.inode_num, 0, buf, 4); //read the first 4 bytes of the file into a buffer
    if((int)buf[0] != EXEC_BYTE_1){
        return 0;
    }
    if((int)buf[1] != EXEC_BYTE_2){
        return 0;
    }
    if((int)buf[2] != EXEC_BYTE_3){
        return 0;
    }
    if((int)buf[3] != EXEC_BYTE_4){
        return 0;
    }   //returns 0 if any of the first 4 bytes don't match up with the ELF notation
    return 1;   //return 1 if executable file
}

/* void set_prog_eip(char* filename)
 * Inputs: char* filename - string holding the file name to get the eip from
 * Return Value: none
 * Function: helper function that gets the running program's eip so that the iret to user knows where to get the program from */
void set_prog_eip(char* filename){
    dentry_t dentry;
    uint32_t file_len;

    read_dentry_by_name((const uint8_t*)filename, &dentry); //get the directory entry of the given file
    file_len = get_file_len(&dentry);
    read_data(dentry.inode_num, 0, ((uint8_t*)(PROG_IMAGE)), file_len);    //read the data of the entire file into the virtual address for programs
    uint32_t* prog_image;
    prog_image = (uint32_t*)PROG_IMAGE_OFFSET;  //offset 24 bytes into the program to get the specific eip of the program
    prog_eip = (uint32_t)(*(prog_image));
}

/* void set_prog_eip(char* filename)
 * Inputs: char* cmd - command to executre, int terminal_id - terminal to execute from
 * Return Value: 0
 * Function: executes on the specific terminal */
int32_t execute_terminal(const char* cmd, int terminal_id){
    int i = 0;
    int z = 0;
    int next;
    int y = 0;
    int x;
    uint8_t arg_buf[ARG_MAX];

    if(cmd == NULL) {
        return -1;
    }

    cli();
    //33 is max filename size
    for(x = 0; x < 34; x++){
        buffer[x] = '\0';   //clearing the copy buffer
        arg_buf[x] = '\0';
    }
    int j;
    while(cmd[i] == '\n' || cmd[i] == '\0' || cmd[i] == ' '){
        i++;
    } 
    while(cmd[i] != '\n' && cmd[i] != '\0' && cmd[i] != ' '){
        buffer[y] = cmd[i]; //filling the copy buffer with the command name
        y++;
        i++;
    }

    while(cmd[i] == '\n' || cmd[i] == '\0' || cmd[i] == ' '){
        i++;
    } 
    while(cmd[i] != '\n' && cmd[i] != '\0' && cmd[i] != ' ') {
        arg_buf[z] = cmd[i];
        z++;
        i++;
    }

    if(!check_executable(buffer)) { //check if command is executable file
        return -1;
    }

    task_avail = 0;

    //can have 6 tasks open at most
    // for(c = 0; c < 6; c++) {
    //     if(!tasks[c]) {
    //         tasks[c] = 1;
    //        // prog_counter = c;
    //         task_avail = 1;
    //         break;
    //     }
    // }
    // if(!task_avail) {
    //     return -1;
    // }
    next = find_available_pid();
    if(next == -1){
        return -1; //no available pids
    }
    
    tasks[next] = 1; //pid is being used

    map_memory(next);   //map virtual to physical memory for new program

    set_prog_eip(buffer);   //find program's eip for iret
 
    pcb_t* pc;
    pcb_t* par_pcb;
    par_pcb = get_pcb_from_pid(pid);
    pc = get_pcb_from_pid(next);   
    pc->saved_ebp = (START_OF_USER-(PCB_SIZE*next)-sizeof(int32_t));
    pc->saved_esp = (START_OF_USER-(PCB_SIZE*next)-sizeof(int32_t));
    
    //get first available PCB from kernel stack
    strcpy((int8_t*)pc->args, (int8_t*)arg_buf);
    pc->active = 1; //set PCB to active
    if(terminal_arr[terminal_id] == -1){
        pc->parent_pid = next;
        pc->parent_ebp = (START_OF_USER-(PCB_SIZE*next)-sizeof(int32_t));
        pc->parent_esp = (START_OF_USER-(PCB_SIZE*next)-sizeof(int32_t));
        
    }
    // if(prog_counter == 0){
    //    // pc->parent_pid = NULL;  //if PCB is the first program, set parent pid to null
    //     pc->parent_pid = 0;
    // }
    else{
        //just update to previous pid
        pc->parent_pid = par_pcb->pid;  //otherwise make parent pid one less than current pid
        pc->parent_esp = par_pcb->saved_esp;
        pc->parent_ebp = par_pcb->saved_ebp;
    }

    pc->pid = next; //set pcb pid to new pid
    terminal_arr[terminal_id] = next;
    //update program variables
    pid = next;
    pc->terminal_idx = terminal_id;

    set_act_terminal(terminal_id); //will need to implement in lib and set up paging

    
    //   prog_counter++; //increment global program counter
    asm volatile ("          \n\
                    movl %%ebp, %%eax  \n\
                    movl %%esp, %%ebx  \n\
            "
            :"=a"(pc->parent_ebp), "=b"(pc->parent_esp)
            );  //saving program's ebp and esp
    pc->fd_arr[0].flag = 1;
    pc->fd_arr[1].flag = 1;
    pc->fd_arr[0].inode_num = 0;
    pc->fd_arr[1].inode_num = 0;
    pc->fd_arr[0].file_position = 0;
    pc->fd_arr[1].file_position = 0;    //initializing the file descriptor array for the first and second entries
    
    init_file_operations(); //calls helper function to initialize all possible file operations
    pc->fd_arr[0].jmp_pointer = &stdin_file_op; 
    pc->fd_arr[1].jmp_pointer = &stdout_file_op;    //set the fd array's function pointer to stdin and stdout for indexes 0 and 1 respectively
    for(j = 2; j < 8; j++){
        pc->fd_arr[j].flag = 0;
        pc->fd_arr[j].inode_num = 0;
        pc->fd_arr[j].file_position = 0;
        pc->fd_arr[j].jmp_pointer = &bad_file_op;
    }   //for the remaining 6 entries fill with bad file operations and set flags low
    tss.esp0 = START_OF_USER - ((pid) * PCB_SIZE) - sizeof(int32_t);    //sets the TSS esp0 to the kernel stack pointer
    pc->saved_ebp = tss.esp0;
    pc->saved_esp = tss.esp0;
    sti();
    goto_user(USER_DS, stack_pointer, eflag_bitmask, USER_CS, prog_eip);    //call asm linkage function for iret (userspace)
    return 0;
}

/* int32_t sys_execute(const char* cmd)
 * Inputs: const char* cmd - string holding the command name to be executed
 * Return Value: return value of halt if sys_halt invoked
 * Function: system call that attempts to load and call a new program */
int32_t sys_execute(const char* cmd){
    pcb_t* curr_pcb = get_pcb(); 
    return execute_terminal(cmd, curr_pcb->terminal_idx);
}

/* void init_file_operations()
 * Inputs: none
 * Return Value: none
 * Function: helper function to intialize all possible file operations */
void init_file_operations(){
    stdin_file_op.open = &terminal_open;
    stdin_file_op.write = &bad_call;
    stdin_file_op.close = &terminal_close;
    stdin_file_op.read = &terminal_read;    //sets stdin file operation to do all terminal operations other than write

    stdout_file_op.open = &terminal_open;
    stdout_file_op.write = &terminal_write;
    stdout_file_op.close = &terminal_close;
    stdout_file_op.read = &bad_call;    //sets stdout file operation to do all terminal operations other than read

    rtc_file_op.open = &rtc_open;
    rtc_file_op.write = &rtc_write;
    rtc_file_op.close = &rtc_close;
    rtc_file_op.read = &rtc_read;   //sets rtc file operation to do all rtc operations

    directory_file_op.open = &open_d;
    directory_file_op.write = &write_d;
    directory_file_op.close = &close_d;
    directory_file_op.read = &read_d;   //sets directory file operation to do all directory operations

    file_file_op.open = &open_f;
    file_file_op.write = &write_f;
    file_file_op.close = &close_f;
    file_file_op.read = &read_f;    //sets file file operation to do all file operations

    bad_file_op.open = &bad_call;
    bad_file_op.write = &bad_call;
    bad_file_op.close = &bad_call;
    bad_file_op.read = &bad_call;   //sets bad call operations to return -1 if needed
}

/* int bad_call()
 * Inputs: none
 * Return Value: -1
 * Function: only returns -1 if needed by an operations call */
int bad_call(){
    return -1;
}

/* pcb_t* get_pcb_from_pid(int pid)
 * Inputs: int pid - current program's pid
 * Return Value: pointer to pcb struct
 * Function: helper function that gets a PCB from memory given a specific program id */
pcb_t* get_pcb_from_pid(int passed_pid){
    pcb_t* i;
    i = (pcb_t*)(START_OF_USER - ((passed_pid+1) * KERNEL_STACK_SIZE));    //calculates place in memory where PCB exists for given pid
    return i;
}

/* pcb_t* get_pcb()
 * Inputs: none
 * Return Value: pointer to current pcb struct
 * Function: helper function that gets the current program's PCB from memory */
pcb_t* get_pcb(){
    pcb_t* cur_pcb;
    cur_pcb = (pcb_t*)(START_OF_USER - ((pid+1) * KERNEL_STACK_SIZE)); //calculates place in memory where current program's PCB exists
    return cur_pcb;
}

int find_available_pid(){
    int i;
    for(i = 0; i < 6; i++){
        if(tasks[i] == 0){
            return i;
        }
    }
    return -1;
}

/* int32_t sys_halt(uint8_t status)
 * Inputs: uint8_t status - how program was halted
 * Return Value: 0
 * Function: system call that terminates a process */
int32_t sys_halt(uint8_t status){
    int i;
    int32_t sesp;
    int32_t sebp;
    uint32_t stat_ret_val;
    pcb_t* cur_pcb = get_pcb_from_pid(pid); //gets the current program's PCB
    pcb_t* parent_pcb = get_pcb_from_pid(cur_pcb->parent_pid);
        if (status == 15) {
        stat_ret_val = 256;
    }
    
    else {
        stat_ret_val = (uint32_t)status;
    }

     //prog_counter--; //decrement program counter
    // if(status == 15) {
    //     status = 256;
    // }
    // else {
    //     status = 0;
    // }
    tasks[cur_pcb->pid] = 0;
    pid = cur_pcb->parent_pid;
    terminal_arr[cur_pcb->terminal_idx] = cur_pcb->parent_pid;
    if(cur_pcb->pid == cur_pcb->parent_pid){  //if current program is the first running one (shell)
        // prog_counter--; //decrement program counter
        clear();
        clear_pos();
        // tasks[prog_counter] = 0;
        // parent_pcb = get_pcb_from_pid(0);
        execute_terminal("shell", cur_pcb->terminal_idx);   //start new shell
    }
    // else{
    //     prog_counter--;
    //     tasks[prog_counter] = 0;
    //     parent_pcb = get_pcb_from_pid(cur_pcb->parent_pid);    //gets the current program's parent's PCB
    // }

    map_memory(parent_pcb->pid);    //re-maps virtual to physical memory for original program

    //size of file descriptor array is 8
    for(i = 0; i < 8; i++){
        if(cur_pcb->fd_arr[i].flag == 1){
            sys_close(i);
        }
        cur_pcb->fd_arr[i].jmp_pointer = &bad_file_op;
        cur_pcb->fd_arr[i].flag = 0;    //set all file operation flags to low
    }
    tss.ss0 = KERNEL_DS;    //set TSS ss0 to kernel data segment
    tss.esp0 = cur_pcb->parent_esp;
  //  tss.esp0 = START_OF_USER - ((parent_pcb->pid) * KERNEL_STACK_SIZE) - sizeof(int32_t);   //sets the TSS esp0 to the kernel stack pointer
    // ret_to_exec(cur_pcb->saved_ebp, cur_pcb->saved_esp, status);    //call asm linkage function for returning the execute sys call 
    sesp = cur_pcb->parent_esp;
    sebp = cur_pcb->parent_ebp;
    asm volatile("          \n\
            movl %0, %%ebp   \n\
            movl %1, %%esp   \n\
            movl %2, %%eax   \n\
            leave               \n\
            ret                 \n\
        "
        :
        :"r"(sebp), "r"(sesp), "r"(stat_ret_val)
        : "cc", "%eax", "%esp", "%ebp"
        );

    return 0;
}

/* int32_t getargs(uint8_t* arguments_buf, int32_t nbytes)
 * Inputs: uint8_t arguments_buf, int32_t nbytes
 * Return Value: 0 on success, -1 on failure
 * Function: sys call to parse arguments passed in from cat command */
int32_t getargs(uint8_t* arguments_buf, int32_t nbytes){
    cli();
    pcb_t* pc_cur = get_pcb_from_pid(pid); //get current pcb

    if(arguments_buf == NULL) return -1; //check if buf is null

    if(nbytes < 0) return -1; //check if nbytes is negative

    if(nbytes > 1024) return -1; //check if nbytes is greater than max arg size

    /* copy args into buf */
    // strncpy((int8_t*)buf,(int8_t*)pc_cur->args, nbytes);
    memcpy(arguments_buf, pc_cur->args, sizeof(pc_cur->args));
    sti();

    return 0;
}

/* int32_t vidmap(uint8_t** screen_start)
 * Inputs: uint8_t** screen_start
 * Return Value: 0
 * Function: maps the text-mode video memory into the user space at a pre-defined virtual address */
int32_t vidmap (uint8_t** screen_start) {
    if(screen_start == NULL) return -1; //check if screen_start is null

    if((uint32_t)screen_start < _128MB || (uint32_t)screen_start > _132MB) return -1; //check if screen_start is within bounds

    set_up_vidmap(); //helper function to set up video page
    flush_tlb();

    *screen_start = (uint8_t*)(_136MB); //sets the screen start address to be at 136 MB 
     
     return 0;
}

/* int32_t sys_open(const uint8_t* filename)
 * Inputs: uint8_t filename - filename passed in to find dentry that contains it
 * Return Value: i - next available fd that is opened, -1 on fail;
 * Function: sys call to find direntry corresponding to filename, allocates an unused file descriptor, and sets up data for given type of file */
int32_t sys_open(const uint8_t* filename){
    int i = 0; // loop idx
    dentry_t dentry; //local dentry obj

    if(filename == NULL || strlen((int8_t*)filename) == 0){ //check if filename is null
        return -1;
    }
    
    if(read_dentry_by_name(filename, &dentry) == -1){  //check if filename is valid and fill dentry object
        return -1;
    }

    pcb_t* pc_cur = get_pcb_from_pid(pid); // get current pcb

    /* loop through each fd starting from 2 to check if open, then fill in, else return -1 if none open */
    for(i = FD_START; i < FD_END; i++){
        if(pc_cur->fd_arr[i].flag == 0){
            pc_cur->fd_arr[i].flag = 1; //mark it as in use
            pc_cur->fd_arr[i].inode_num = dentry.inode_num;
            pc_cur->fd_arr[i].file_position = 0;
            switch (dentry.filetype)
            {
            case 0:
                /* code to say file is rtc, use jmp pointer */
                pc_cur->fd_arr[i].jmp_pointer = &rtc_file_op;
                pc_cur->fd_arr[i].jmp_pointer->open(filename);
                return i; // return the next available fd
            case 1:
                /* code to say file is dir, use jmp pointer */
                pc_cur->fd_arr[i].jmp_pointer = &directory_file_op;
                pc_cur->fd_arr[i].jmp_pointer->open(filename);
                return i; // return the next available fd
            case 2:
                /* code to say file is file, use jmp pointer */
                pc_cur->fd_arr[i].jmp_pointer = &file_file_op;
                pc_cur->fd_arr[i].jmp_pointer->open(filename);
                return i; // return the next available fd
            default:
                return -1;
            }
        }
    }
    return -1; //no file descriptor free
}

/* int32_t sys_read(int32_t fd, void *buf, int32_t nbytes)
 * Inputs: uint8_t fd - file descriptor
           void *buf  - buffer to hold nbytes read
           int32_t nbytes - number of bytes to be read
 * Return Value: number of bytes read, -1 on failure;
 * Function: sys call to read data from the keyboard, a file, device(RTC), or directory */
int32_t sys_read(int32_t fd, void *buf, int32_t nbytes){
    pcb_t* pc_cur = get_pcb_from_pid(pid); //get current pcb

    if(fd < 0 || fd > FD_MAX) return -1; //invalid fd index

    if(pc_cur->fd_arr[fd].flag == 0) return -1;

    if(fd == STDOUT) return -1; // return -1 for stdout on read
    //if(pc_cur->fd_arr[fd])
    return pc_cur->fd_arr[fd].jmp_pointer->read(fd, buf, nbytes); //call read 
}

/* int32_t sys_write(int32_t fd, void *buf, int32_t nbytes)
 * Inputs: uint8_t fd - file descriptor
           void *buf  - buffer to hold nbytes written
           int32_t nbytes - number of bytes to be written
 * Return Value: number of bytes written, -1 on failure;
 * Function: sys call to write data to the terminal or to a device(RTC) */
int32_t sys_write(int32_t fd, const void *buf, int32_t nbytes){
    if(fd < FD_MIN || fd > FD_MAX) return -1; //invalid fd index

    pcb_t* pc_cur = get_pcb_from_pid(pid); //get current pcb

    //if(pc_cur->fd_arr[fd].flag == 0) return -1; // return -1 if fd in use
    if(fd == STDIN) return -1;

    return pc_cur->fd_arr[fd].jmp_pointer->write(fd, buf, nbytes); //call write
}

/* int32_t sys_close(int32_t fd, void *buf, int32_t nbytes)
 * Inputs: uint8_t fd - file descriptor
 * Return Value: 0 for successful close, -1 for failure;
 * Function: sys call to close the specified file descriptor and make it available for return from later calls to open */
int32_t sys_close(int32_t fd){
    if(fd == STDIN || fd == STDOUT){ //cant close stdout or stdin so return -1
        return -1;
    }
    if(fd < FD_MIN || fd > FD_MAX){ //check if fd is within bounds
        return -1;
    }

    pcb_t* pc_cur = get_pcb_from_pid(pid); //get current pcb

    /* change fd status from unavailable to available */
    if(pc_cur->fd_arr[fd].flag == 0){
        return -1;
    }
    pc_cur->fd_arr[fd].flag = 0;
    if(0 != pc_cur->fd_arr[fd].jmp_pointer->close(fd)){
        return -1;
    }
    return 0; //return -1 if trying to close an fd that that is unallocated
}
