#include "paging.h"
#include "x86_desc.h"
#include "lib.h"
#include "sysCalls.h"

#define VIDEO       0xB8000 //video memory address in physical memory
#define START_OF_USER   0x800000
#define START_OF_KERNEL 0x400000
#define VIDEO_MEM_INDEX 184
#define NUM_ENTRIES 1024
#define START_OF_KERNEL 0x400000
#define VIDEO_DIR_INDEX 34
#define PAGE_VIDEO 0xB8
extern void loadPageDirectory(uint32_t* page_dir);
extern void enablePaging();
extern void flush_tlb();

page_directory_entry_t pde[NUM_ENTRIES] __attribute__((aligned(4096))); //aligned every 4mb
page_table_entry_t pte[NUM_ENTRIES] __attribute__((aligned(4096))); //aligned every 4kb
page_table_entry_t pte_vidmap[NUM_ENTRIES] __attribute__((aligned(4096)));


void page_init(){
    int i;
    /* generically fill Page Table */
    for(i = 0; i < NUM_ENTRIES; i++){
        // if(i * 4096 == VIDEO){
        //     pte[i].present = 1;
        // }
        // else{
        //     pte[i].present = 0;
        // }
        pte[i].present = 0;
        pte[i].read_write = 1;
        pte[i].user_supervisor = 0;
        pte[i].write_through = 0;
        pte[i].cache_disable = 0;
        pte[i].accessed = 0;
        pte[i].dirty = 0;
        pte[i].page_attribute_table = 0;
        pte[i].global = 0;
        pte[i].available = 0x00;
        pte[i].page_base_addr = 0x00000;
    }
    /* fill video memory page in page table */
    pte[VIDEO_MEM_INDEX].page_base_addr = (uint32_t) VIDEO >> 12; //shift from 32 bits to 20 bits
    pte[VIDEO_MEM_INDEX].read_write = 1;
    pte[VIDEO_MEM_INDEX].user_supervisor = 0;
    pte[VIDEO_MEM_INDEX].write_through = 0;
    pte[VIDEO_MEM_INDEX].cache_disable = 1;
    pte[VIDEO_MEM_INDEX].accessed = 0;
    pte[VIDEO_MEM_INDEX].dirty = 0;
    pte[VIDEO_MEM_INDEX].page_attribute_table = 0;
    pte[VIDEO_MEM_INDEX].global = 0;
    pte[VIDEO_MEM_INDEX].available = 0x00;
    pte[VIDEO_MEM_INDEX].present = 1;

    pte[VIDEO_MEM_INDEX+1].page_base_addr = ((uint32_t) VIDEO >> 12) + 1; //shift from 32 bits to 20 bits
    pte[VIDEO_MEM_INDEX+1].read_write = 1;
    pte[VIDEO_MEM_INDEX+1].user_supervisor = 0;
    pte[VIDEO_MEM_INDEX+1].write_through = 0;
    pte[VIDEO_MEM_INDEX+1].cache_disable = 1;
    pte[VIDEO_MEM_INDEX+1].accessed = 0;
    pte[VIDEO_MEM_INDEX+1].dirty = 0;
    pte[VIDEO_MEM_INDEX+1].page_attribute_table = 0;
    pte[VIDEO_MEM_INDEX+1].global = 0;
    pte[VIDEO_MEM_INDEX+1].available = 0x00;
    pte[VIDEO_MEM_INDEX+1].present = 1;

    pte[VIDEO_MEM_INDEX+2].page_base_addr = ((uint32_t) VIDEO >> 12) + 2; //shift from 32 bits to 20 bits
    pte[VIDEO_MEM_INDEX+2].read_write = 1;
    pte[VIDEO_MEM_INDEX+2].user_supervisor = 0;
    pte[VIDEO_MEM_INDEX+2].write_through = 0;
    pte[VIDEO_MEM_INDEX+2].cache_disable = 1;
    pte[VIDEO_MEM_INDEX+2].accessed = 0;
    pte[VIDEO_MEM_INDEX+2].dirty = 0;
    pte[VIDEO_MEM_INDEX+2].page_attribute_table = 0;
    pte[VIDEO_MEM_INDEX+2].global = 0;
    pte[VIDEO_MEM_INDEX+2].available = 0x00;
    pte[VIDEO_MEM_INDEX+2].present = 1;

    pte[VIDEO_MEM_INDEX+3].page_base_addr = ((uint32_t) VIDEO >> 12) + 3; //shift from 32 bits to 20 bits
    pte[VIDEO_MEM_INDEX+3].read_write = 1;
    pte[VIDEO_MEM_INDEX+3].user_supervisor = 0;
    pte[VIDEO_MEM_INDEX+3].write_through = 0;
    pte[VIDEO_MEM_INDEX+3].cache_disable = 1;
    pte[VIDEO_MEM_INDEX+3].accessed = 0;
    pte[VIDEO_MEM_INDEX+3].dirty = 0;
    pte[VIDEO_MEM_INDEX+3].page_attribute_table = 0;
    pte[VIDEO_MEM_INDEX+3].global = 0;
    pte[VIDEO_MEM_INDEX+3].available = 0x00;
    pte[VIDEO_MEM_INDEX+3].present = 1;

    pte[VIDEO_MEM_INDEX+4].page_base_addr = ((uint32_t) VIDEO >> 12) + 4; //shift from 32 bits to 20 bits
    pte[VIDEO_MEM_INDEX+4].read_write = 1;
    pte[VIDEO_MEM_INDEX+4].user_supervisor = 0;
    pte[VIDEO_MEM_INDEX+4].write_through = 0;
    pte[VIDEO_MEM_INDEX+4].cache_disable = 1;
    pte[VIDEO_MEM_INDEX+4].accessed = 0;
    pte[VIDEO_MEM_INDEX+4].dirty = 0;
    pte[VIDEO_MEM_INDEX+4].page_attribute_table = 0;
    pte[VIDEO_MEM_INDEX+4].global = 0;
    pte[VIDEO_MEM_INDEX+4].available = 0x00;
    pte[VIDEO_MEM_INDEX+4].present = 1;

    /* generically fill Page Directory */
    for(i = 0; i < NUM_ENTRIES; i++){
        pde[i].present = 0;
        pde[i].read_write = 1;
        pde[i].user_supervisor = 0;
        pde[i].write_through = 0;
        pde[i].cache_disable = 0;
        pde[i].accessed = 0;
        pde[i].dirty = 0;
        pde[i].page_size = 0;
        pde[i].global = 0;
        pde[i].available = 0x00;
        pde[i].page_table_base_addr = 0x00000;
    }
    /*generically fill vidmap Page Table*/
    for(i = 0; i < NUM_ENTRIES; i++){
        pte_vidmap[i].page_base_addr = i;
        pte_vidmap[i].present = 0;
        pte_vidmap[i].read_write = 1;
        pte_vidmap[i].user_supervisor = 0;
        pte_vidmap[i].write_through = 0;
        pte_vidmap[i].cache_disable = 1;
        pte_vidmap[i].accessed = 0;
        pte_vidmap[i].dirty = 0;
        pte_vidmap[i].page_attribute_table = 0;
        pte_vidmap[i].global = 0;
        pte_vidmap[i].available = 0x00;
    }
    /* fill video memory table in page directory */
    pde[0].page_table_base_addr = ((uint32_t) pte) >> 12; //shift from 32 bits to 20 bits
    pde[0].read_write = 1;
    pde[0].user_supervisor = 0;
    pde[0].write_through = 0;
    pde[0].cache_disable = 0;
    pde[0].accessed = 0;
    pde[0].dirty = 0;
    pde[0].page_size = 0;
    pde[0].global = 0;
    pde[0].available = 0x00;
    pde[0].present = 1;

    /* fill kernel page in page directory */
    pde[1].page_table_base_addr = ((uint32_t) START_OF_KERNEL) >> 12; //shift from 32 bits to 20 bits
    pde[1].read_write = 1;
    pde[1].user_supervisor = 0;
    pde[1].write_through = 0;
    pde[1].cache_disable = 0;
    pde[1].accessed = 0;
    pde[1].dirty = 0;
    pde[1].page_size = 1;
    pde[1].global = 0;
    pde[1].available = 0x00;
    pde[1].present = 1;

    /* load page directory and enable paging for 4MB page and 4kB page*/
    loadPageDirectory((uint32_t* )pde);
    enablePaging();
}

/* void set_up_vidmap()
 * Inputs: None
 * Return Value: None
 * Function: Helper function to be used in vidmap to set up the vidmap paging */
void set_up_vidmap(){
    /*set up pde at 136 MB*/
    pde[VIDEO_DIR_INDEX].present = 1;
    pde[VIDEO_DIR_INDEX].page_size = 0;
    pde[VIDEO_DIR_INDEX].user_supervisor = 1;
    pde[VIDEO_DIR_INDEX].page_table_base_addr = ((uint32_t)pte_vidmap) >> 12; //shift from 32 bits to 20 bits

    /*set up a page of the vidmap page table*/
    pte_vidmap[0].present = 1;
    pte_vidmap[0].user_supervisor = 1;
    pte_vidmap[0].page_base_addr = VIDEO >> 12;
}


/* void set_up_pid_map()
 * Inputs: int pid -- pid number to set up
 * Return Value: None
 * Function: Helper function to be used in pid to set up the pid paging */
void set_up_pid_map(int pid){

    pde[32].present = 1;
    pde[32].page_size = 1;
    pde[32].user_supervisor = 1;
    pde[32].read_write = 1;
    pde[32].page_table_base_addr = ((pid * START_OF_KERNEL) + START_OF_USER)>> 12; //multiple by 4mb and add 8 mb to get to the correct page directory entry
    flush_tlb();
}

/* void set_up_vidmap()
 * Inputs: int vir_addr -- virtual address to write to, int term -- termial to write to
 * Return Value: None
 * Function: Helper function to be used in vidmap to set up the vidmap paging for a specific terminal */
void set_up_vidmap_terminals(int vir_addr, int term){
    int pd_idx;
    int terminal_display = get_visible_terminal();
    pd_idx = vir_addr >> 22;

    page_directory_entry_t vidmap_entry;
    vidmap_entry.present = 0;
    vidmap_entry.read_write = 0;
    vidmap_entry.user_supervisor = 0;
    vidmap_entry.write_through = 0;
    vidmap_entry.cache_disable = 0;
    vidmap_entry.accessed = 0;
    vidmap_entry.dirty = 0;
    vidmap_entry.page_size = 0;
    vidmap_entry.global = 0;
    vidmap_entry.available = 0;
    vidmap_entry.page_table_base_addr = 0;

    pde[pd_idx] = vidmap_entry;

    pde[pd_idx].present = 1;
    pde[pd_idx].user_supervisor = 1;
    pde[pd_idx].read_write = 1;
    pde[pd_idx].page_table_base_addr = (uint32_t)pte_vidmap >> 12;

    pte_vidmap[0].present = 1;
    pte_vidmap[0].read_write = 1;
    pte_vidmap[0].user_supervisor = 1;
    pte_vidmap[0].write_through = 1;
    pte_vidmap[0].cache_disable = 0;
    pte_vidmap[0].accessed = 0;
    pte_vidmap[0].dirty = 0;
    pte_vidmap[0].page_attribute_table = 0;
    pte_vidmap[0].global = 0;
    pte_vidmap[0].available = 0;

    if(term == terminal_display){
        pte_vidmap[0].page_base_addr = PAGE_VIDEO;
    }
    else{
        pte_vidmap[0].page_base_addr = 0;
    }
    flush_tlb();
}

/* void void set_active_paging
 * Inputs: None
 * Return Value: None
 * Function: Helper function to update active termianl paging */
void set_active_paging(){
    pte[PAGE_VIDEO].page_base_addr = PAGE_VIDEO;
    flush_tlb();
}
