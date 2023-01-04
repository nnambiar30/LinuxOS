/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */


/* void i8259_init(void);
 * Inputs: void
 * Return Value: none
 * Function: Initializes PIC */
void i8259_init(void) {
    
    outb(0xFF, MASTER_8259_DATA_PORT); /* mask all of the master and slave ports*/
    outb(0xFF, SLAVE_8259_DATA_PORT);

    master_mask = slave_mask = 0xFF; //clears slave and master mask

    outb(ICW1, MASTER_8259_COMMAND_PORT); /*starts the initialization of the master in cascade mode*/
    outb(ICW2_MASTER, MASTER_8259_DATA_PORT); /*IRQ:0-7 mapped to ports x20-27*/
    outb(ICW3_MASTER, MASTER_8259_DATA_PORT); /*tells the master PIC that the slave is connected at IRQ2*/
    outb(ICW4, MASTER_8259_DATA_PORT);

    outb(ICW1, SLAVE_8259_COMMAND_PORT); /* starts the initialization of the slave in cascade mode*/
    outb(ICW2_SLAVE, SLAVE_8259_DATA_PORT); /*IRQ:8-15 mapped tp x28-2f*/
    outb(ICW3_SLAVE, SLAVE_8259_DATA_PORT); /*tells slave that it is connected to master*/
    outb(ICW4, SLAVE_8259_DATA_PORT);

    

    enable_irq(2); // enables slave pic irq


}
/* void enable_irq(uint32_t irq_num);
 * Inputs: uint32_t irq_num specifies which irq to enable
 * Return Value: none
 * Function: enables irq based on irq_num*/
void enable_irq(uint32_t irq_num) {
    // out of bounds checks (0-15)
    if(irq_num > 15 || irq_num < 0) {
        return;
    }
    
    /*enable slave irq*/
    if(irq_num >= 8) {
        irq_num -= 8;
        slave_mask = slave_mask & ~ (1 << irq_num); 
        outb(slave_mask, SLAVE_8259_DATA_PORT);
    }
    /*enable mater irq*/
    else {
        master_mask = master_mask & ~ (1 << irq_num);
        outb(master_mask, MASTER_8259_DATA_PORT);
    }
}

/* void disable_irq(uint32_t irq_num);
 * Inputs: uint32_t irq_num specifies which irq to disable
 * Return Value: none
 * Function: disable irq based on irq_num*/
void disable_irq(uint32_t irq_num) {
    // out of bounds checks (0-15)
    if(irq_num > 15 || irq_num < 0) {
        return;
    }

    /*disable slave_irq*/
    if(irq_num >= 8) {
        irq_num -= 8;
        slave_mask = slave_mask | (1<< irq_num);
        outb(slave_mask, SLAVE_8259_DATA_PORT);
    }
    /*disable master_irq*/
    else {
        master_mask = master_mask | (1 << irq_num);
        outb(master_mask, MASTER_8259_DATA_PORT);
    }
}

/* void send_eoi(uint32_t irq_num);
 * Inputs: uint32_t irq_num specifies which irq to send end of interrupt
 * Return Value: none
 * Function: sends end of interrupt to irq based on irq_num*/
void send_eoi(uint32_t irq_num) {
    // out of bounds checks (0-15)
    if(irq_num > 15 || irq_num < 0) {
        return;
    }
    /*send eoi to slave*/
    if(irq_num >= 8) {
        outb(EOI | (irq_num-8), SLAVE_8259_COMMAND_PORT);
        outb (EOI | (0x02), MASTER_8259_COMMAND_PORT);
    }
    /*send eoi to master*/
    else {
        outb(EOI | irq_num, MASTER_8259_COMMAND_PORT); 
    }
    return;
} 
