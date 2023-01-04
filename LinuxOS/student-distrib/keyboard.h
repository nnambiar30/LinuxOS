#ifndef _KEYBOARD_H
#define _KEYBOARD_H

extern void keyboard_init(void);
extern void keyboard_handler(void);
void kb_newline(char* buffer);
extern char kb_buffer[3][128];
extern int t1_read, t2_read, t3_read;
extern char term_buffer[3][2050];
extern int visible_term;
extern int count[3];
extern int clear_screen;
extern int enter_pressed[3];
extern int tab_flag;

#endif /* _KEYBOARD_H */
