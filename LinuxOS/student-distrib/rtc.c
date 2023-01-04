
#include "lib.h"
#include "i8259.h"
#include "rtc.h"

//FLAGS
volatile int BLOCK_FLAG = 0; //flag to tell read to block until next interrupt


// unsigned int Log2n(unsigned int n);

#define RTC_PORT 0x70 //RTC port
#define RTC_DATA 0x71 //RTC data port
#define RTC_REG_A 0x8A //RTC register A
#define RTC_REG_B 0x0B //RTC register B

/* void rtc_init(void);
 * Inputs: void
 * Return Value: none
 * Function: Initializes rtc */
void rtc_init(void) {
    /* sets the registers for the rtc */
    cli();
    outb(RTC_REG_B, RTC_PORT); //Select register B
    char prev = inb(RTC_DATA); //read value at register B
    outb(RTC_REG_B, RTC_PORT); //set the index again
    outb(prev | 0x40, RTC_DATA);  //turn on bit 6 of reigster B
    outb(RTC_REG_A, RTC_PORT); //disable NMI
    outb(0x06, RTC_DATA); //set frequency to 1024
    enable_irq(8); //RTC at irq 8
    sti();
}


/* void rtc_handler(void);
 * Inputs: void
 * Return Value: none
 * Function: Handles rtc interrupt and sends EOI*/
void rtc_handler() {
    outb(0x0C, RTC_PORT); //select register C
    inb(RTC_DATA); //throw away contents
    //test_interrupts();
    BLOCK_FLAG = 1;
    send_eoi(8);
}

/* void rtc_open(void);
 * Inputs: void
 * Return Value: none
 * Function: resets rtc to 2hz*/
int rtc_open(const uint8_t* filename){
    cli();
    outb(RTC_REG_A, RTC_PORT);		// set index to register A, disable NMI
    char prev=inb(RTC_DATA);	// get initial value of register A
    outb(RTC_REG_A, RTC_PORT);		// reset index to A
    outb((prev & 0xF0) | 0x0F, RTC_DATA); //write only our rate to A. Note, rate is 0x0F, which sets rate to 2hz
    sti();
    return 0;
}

/* void rtc_read(int32_t fd, void *buf, int32_t nbytes);
 * Inputs: int32_t fd - file data
           void *buf - buffer that hold frequency
           int32_t nbytes - N/A
 * Return Value: none
 * Function: blocks until next interrupt*/
int rtc_read(int32_t fd, void *buf, int32_t nbytes){
    BLOCK_FLAG = 0;
    while(!BLOCK_FLAG);
    BLOCK_FLAG = 1;
    //outb(0x0C, 0x70);	// select register C
    //inb(RTC_DATA);		// just throw away contents
    return 0; 
}

/* int rtc_write(int32_t fd, const void *buf, int32_t nbytes)
 * Inputs: int32_t fd - file data
 *         const void *buf - buffer that holds frequency
 *         in32_t nbytes - N/A
 * Return Value: none
 * Function: writes new frequency from the buffer to the RTC 
 */ 
int rtc_write(int32_t fd, const void *buf, int32_t nbytes){
    //int target;
    //int rate;
    int new_rate;
    //int max_freq = 32768; //max frequency value
    int* result = (int*)buf;
    int freq = *result;

    int hz_2 = 2;
    int hz_4 = 4;
    int hz_8 = 8;
    int hz_16 = 16;
    int hz_32 = 32;
    int hz_64 = 64;
    int hz_128 = 128;
    int hz_256 = 256;
    int hz_512 = 512;
    int hz_1024 = 1024;

    if(freq == NULL || ((freq & (freq - 1)) != 0)){ // check if freq is a power of 2 and is not null 
        return -1;
    }

    if(freq < 2 || freq > 1024){
        return -1;
    }

    /*check for each rate if the target frequency is equal to the new frequency, then set the new rate*/
    // for(rate = 6; rate < 17; rate++){
    //     target = max_freq >> (rate - 1);
    //     /*set new rate*/
    //     if(target == freq){
    //         new_rate = rate - 1;
    //         break;
    //     }
    // }

    if(freq == hz_2){
        new_rate = 15; // sets rtc to 2 hz 
    }
    if(freq == hz_4){
        new_rate = 14; // sets rtc to 4 hz
    }
    if(freq == hz_8){
        new_rate = 13; // sets rtc to 8 hz
    }
    if(freq == hz_16){
        new_rate = 12; // sets rtc to 16 hz
    }
    if(freq == hz_32){
        new_rate = 11; // sets rtc to 32 hz
    }
    if(freq == hz_64){
        new_rate = 10; // sets rtc to 64 hz
    }
    if(freq == hz_128){
        new_rate = 9; // sets rtc to 128 hz
    }
    if(freq == hz_256){
        new_rate = 8; // sets rtc to 256 hz
    }
    if(freq == hz_512){
        new_rate = 7; // sets rtc to 512 hz
    }
    if(freq == hz_1024){
        new_rate = 6; // sets rtc to 1024 hz
    }





    
    /*check if new rate is within bounds*/
    // if(new_rate < 6){
    //     new_rate = 6;
    // }

    /*set new frequency*/
    cli();
    outb(RTC_REG_A, RTC_PORT);		// set index to register A, disable NMI
    char prev=inb(RTC_DATA);	// get initial value of register A
    outb(RTC_REG_A, RTC_PORT);		// reset index to A
    outb((prev & 0xF0) | new_rate, RTC_DATA); //write only our rate to A. Note, rate is 0x0F, which sets rate to freq
    sti();

    return nbytes;
}

/* void rtc_cclose(int32_t fd);
 * Inputs: int32_t fd - file data
 * Return Value: none
 * Function: does nothing*/
int rtc_close(int32_t fd){
    return 0;
}
