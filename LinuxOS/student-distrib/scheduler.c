
#include "lib.h"
#include "i8259.h"
#include "x86_desc.h"
#include "paging.h"
#include "terminal.h"
#include "sysCalls.h"
#include "scheduler.h"

#define _8MB 0x800000
#define PCB_SIZE 0x1000
#define _1MB 0x100000
#define _132MB 0x8400000

int8_t prog_timer = 0;
int terminal_arr[MAX_TERMINALS];

/* int32_t get_next_terminal
 * Inputs: int32_t curr 
 * Return Value: the next terminal
 * Function: calculates which terminal to switch to*/
int32_t get_next_terminal(int32_t curr){
    int32_t next_terminal = (curr + 1) % MAX_TERMINALS;
    return next_terminal;
}

/* void pit_init
 * Inputs: none 
 * Return Value: none
 * Function: initializes pit*/
void pit_init(void){
    //set the frequency of the PIT
    enable_irq(PIT_IRQ); //enable the PIT
    outb(PIT_CMD, PIT_CMD_PORT); //set the command byte
    outb((PIT_FREQ/TAR_FREQ) & LSB_MASK, PIT_PORT); //set the LSB
    outb((PIT_FREQ/TAR_FREQ) >> MSB_SHIFT, PIT_PORT); //set the MSB
}

/* void pit_init
 * Inputs: none 
 * Return Value: none
 * Function: handles the pit interrupts*/
void pit_handler(void){
    int next_term;
    pcb_t *cur_pcb;
    pcb_t *next_pcb;

    cli();
    //send EOI
    send_eoi(PIT_IRQ);

    if(terminal_arr[0] == -1){ //make new terminal if all are closed
        clear();
        clear_pos();
        execute_terminal("shell", 0);
    } 

    cur_pcb = get_pcb_from_pid(pid); //get current pcb
    next_term = get_next_terminal(cur_pcb->terminal_idx);


    while(terminal_arr[next_term] == -1){  // gets next termianl and update other terminals
        next_term = get_next_terminal(next_term);
    }

    next_pcb = get_pcb_from_pid(terminal_arr[next_term]);

    set_up_pid_map(next_pcb->pid); //sets up pid map

    set_up_vidmap_terminals(_132MB, next_pcb->terminal_idx); //sets up vidmap
    

    tss.ss0 = KERNEL_DS; //sets up stack
    tss.esp0 = (_8MB-(PCB_SIZE*next_pcb->pid)-sizeof(int32_t)); // sets up esp
    pid = next_pcb->pid;
    set_act_terminal(next_pcb->terminal_idx); //set active terminal


    sti();

    asm volatile ("          \n\
                 movl %%ebp, %0  \n\
                 movl %%esp, %1  \n\
            "
            :"=r"(cur_pcb->saved_ebp), "=r"(cur_pcb->saved_esp)
            );

    //load in next process esp and ebp
    asm volatile ("          \n\
                 movl %0, %%esp  \n\
                 movl %1, %%ebp  \n\
            "
            : :"r"(next_pcb->saved_esp), "r"(next_pcb->saved_ebp)
            );


    return;
}
