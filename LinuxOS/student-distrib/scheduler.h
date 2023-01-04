#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#define MAX_PROCESSES 6
#define MAX_TERMINALS 3
#define MSB_SHIFT 8
#define LSB_MASK 0xFF
#define TAR_FREQ 50
#define PIT_FREQ 1193180
#define PIT_IRQ 0
#define PIT_PORT 0x40
#define PIT_CMD_PORT 0x43
#define PIT_CMD 0x36

extern int terminal_arr[MAX_TERMINALS];
extern void pit_init(void);
extern void pit_handler(void);

#endif /* _SCHEDULER_H */
