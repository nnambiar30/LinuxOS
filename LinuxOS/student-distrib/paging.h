#ifndef _PAGING_H
#define _PAGING_H

extern void page_init(void);
extern void set_up_vidmap(void);
extern void set_up_pid_map(int pid);
extern void set_active_paging();
extern void set_up_vidmap_terminals(int vir_addr, int term);

#endif /* _PAGING_H */
