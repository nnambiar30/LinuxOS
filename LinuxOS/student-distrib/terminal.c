#include "keyboard.h"
#include "lib.h"
#include "terminal.h"

#define MAX_BUFFER_SIZE 128

/* int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: int32_t fd - file data
           void* buf - Buffer that will copy the keyboard buffer
           int32_t nbytes - number of bytes to be read
 * Return Value: number of bytes that were read
 * Function: when enter is pressed, reads the data from the keyboard buffer into the general buffer */
int32_t terminal_read (int32_t fd, void* buf, int32_t nbytes){
    sti();
    if(buf == NULL) {   //return -1 if buffer is null
        return -1;
    }
    if(nbytes <= 0) {   //return 0 if no bytes are to be read
        return 0;
    }
    if(nbytes > MAX_BUFFER_SIZE){
        nbytes = MAX_BUFFER_SIZE;   //limit the amount of bytes to be read to max buffer size (128 chars)
    }

    while(!enter_pressed[visible_term]){} //do nothing if enter is not pressed

    char* result = (char*)buf;
    int act_term = get_act_terminal();  //gets currently active terminal

    memcpy(result, term_buffer[act_term], nbytes);  //copy terminal buffer into buffer

    enter_pressed[act_term] = 0;  //reset enter flag and index for keyboard buffer
    count[visible_term] = 0;
    return strlen(result);  //return number of bytes read
}

/* int32_t terminal_write(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: int32_t fd - file data
           void* buf - Buffer that will echo to the screen
           int32_t nbytes - number of bytes to be written
 * Return Value: number of bytes that were written
 * Function: writes from the buffer to the screen whenever it is populated */
int32_t terminal_write (int32_t fd, const void* buf, int32_t nbytes){
    cli();  //disable interrupts
    if(buf == NULL) {
        sti();
        return -1;  //checks if buffer is null
    }
    if(nbytes <= 0) {
        sti();
        return 0;   //return 0 if no bytes are to be written
    }
    char* result = (char*)buf;
    int i = 0;
    int act_term = get_act_terminal();  //gets currently active terminal

    int copy_limit = sizeof(term_buffer[act_term]);
    if(nbytes < copy_limit){
        copy_limit = nbytes;    //adjusts the amount of bytes to be copied
    }
    memcpy(term_buffer[act_term], result, copy_limit);  //copy the buffer into the printed buffer
    term_buffer[act_term][copy_limit] = 0;
    for(i = 0; i < copy_limit; i++){
        if(term_buffer[act_term][i] == '\0') {  //ignore null chars
            continue;
        }
        putc(term_buffer[act_term][i]); //put printed buffer onto screen
    }
    sti();  //enable interrupts
    return nbytes;  //return number of bytes written
}

/* int32_t terminal_open(const uint8_t* filename)
 * Inputs: const uint8_t* filename - filename to open
 * Return Value: nothing
 * Function: initialize terminal stuff or do nothing */
int32_t terminal_open (const uint8_t* filename){
    return 0;
}

/* int32_t terminal_close(int32_t fd)
 * Inputs: int32_t fd - file data
 * Return Value: nothing
 * Function: clear terminal stuff or do nothing */
int32_t terminal_close (int32_t fd){
    return 0;
}
