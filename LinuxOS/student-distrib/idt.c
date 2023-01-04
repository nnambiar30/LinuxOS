#include "idt.h"
#include "x86_desc.h"
#include "lib.h"

extern void asm_link_kb(void);
extern void asm_link_rtc(void);
extern void asm_link_pit(void);
extern void sys_call_handler(void);

/* void div_by_zero_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: raises div by zero exception*/
void div_by_zero_exception(){
    clear();
    printf(" divide by zero exception raised ");
    while(1);
}

/* void debug_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: raises debug exception*/
void debug_exception(){
    clear();
    printf(" debug exception raised ");
    while(1);
}

/* void nmi_interrupt(void);
 * Inputs: void
 * Return Value: none
 * Function: raises nmi interrupt*/
void NMI_exception(){
    clear();
    printf(" non-maskable interrupt exception raised ");
    while(1);
}

/* void breakpoint_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: raises breakpoint exception*/
void breakpoint_exception(){
    clear();
    printf(" breakpoint exception raised ");
    while(1);
}

/* void overflow_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: raises overflow exception*/
void overflow_exception(){
    clear();
    printf(" overflow exception raised ");
    while(1);
}

/* void bounds_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: raises bounds exception*/
void bound_range_exceeded_exception(){
    clear();
    printf(" bound range exceeded exception raised ");
    while(1);
}

/* void invalid_opcode_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: raises invalid opcode exception*/
void invalid_opcode_exception(){
    clear();
    printf(" invalid opcode exception raised ");
    while(1);
}

/* void device_not_available_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: raises device not available exception*/
void device_not_available_exception(){
    clear();
    printf(" device not available exception raised ");
    while(1);
}

/* void double_fault_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: raises double fault exception*/
void double_fault_exception(){
    clear();
    printf(" double fault exception raised ");
    while(1);
}

/* void coprocessor_segment_overrun(void);
 * Inputs: void
 * Return Value: none
 * Function: raises coprocessor segment overrun exception*/
void coprocessor_segment_overrun(){
    clear();
    printf(" coprocessor segment overrun exception raised ");
    while(1);
}

/* void invalid_tss_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: raises invalid tss exception*/
void invalid_TSS_exception(){
    clear();
    printf(" invalid TSS exception raised ");
    while(1);
}

/* void segment_not_present_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: raises segment not present exception*/
void segment_not_present_exception(){
    clear();
    printf(" segment not present exception raised ");
    while(1);
}

/* void stack_segment_fault_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: raises stack segment fault exception*/
void stack_fault_exception(){
    clear();
    printf(" stack fault exception raised ");
    while(1);
}

/* void general_protection_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: raises general protection exception*/
void general_protection_exception(){
    clear();
    printf(" general protection exception raised ");
    while(1);
}

/* void page_fault_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: raises page fault exception*/
void page_fault_exception(){
    clear();
    printf(" page-fault exception raised ");
    while(1);
}

/* void x87_FPU_fp_error(void);
 * Inputs: void
 * Return Value: none
 * Function: raises floating point exception*/
void x87_FPU_fp_error(){
    clear();
    printf(" x87 FPU floating-point exception raised ");
    while(1);
}

/* void alignment_check_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: raises alignment check exception*/
void alignment_check_exception(){
    clear();
    printf(" alignment check exception raised ");
    while(1);
}

/* void machine_check_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: raises machine check exception*/
void machine_check_exception(){
    clear();
    printf(" machine check exception raised ");
    while(1);
}

/* void SIMD_fp_exception(void);
 * Inputs: void
 * Return Value: none
 * Function: raises SIMD floating point exception*/
void SIMD_fp_exception(){
    clear();
    printf(" SIMD Floating-Point exception raised ");
    while(1);
}

/* void system_call(void);
 * Inputs: void
 * Return Value: none
 * Function: raises system_call*/
void system_call(){
    clear();
    printf(" System Call raised ");
    while(1);
}

/* void initialize_idt(void);
 * Inputs: void
 * Return Value: none
 * Function: Initialzies idt. Sets all the bits for idt to corresponding value */
void initialize_idt(){
    int i;
    for(i = 0; i < NUM_VEC; i++){
        if(i != 15){ //15 is reserved by intel
            idt[i].present = 0x1; 
        }
        else{
            idt[i].present = 0x0;
        }
        if(i == 0x80){
            idt[i].dpl = 0x3;   //sets dpl to 3 if idt entry is a system call
        }
        else{
            idt[i].dpl = 0x0;   //otherwise sets dpl to 0
        }
        idt[i].size = 0x1;
        idt[i].reserved0 = 0x0;
        idt[i].reserved1 = 0x1;
        idt[i].reserved2 = 0x1;
        if((i >= 0x20) && (i <= 0x2F)){ //sets reserved3 bit to 0 if idt entry is an interrupt
            idt[i].reserved3 = 0x0;
        }
        else{
            idt[i].reserved3 = 0x1; //otherwise set reserved3 bit to 1
        }
        idt[i].reserved4 = 0x0;
        idt[i].seg_selector = KERNEL_CS;
    }
    SET_IDT_ENTRY(idt[0], div_by_zero_exception);
    SET_IDT_ENTRY(idt[1], debug_exception);
    SET_IDT_ENTRY(idt[2], NMI_exception);
    SET_IDT_ENTRY(idt[3], breakpoint_exception);
    SET_IDT_ENTRY(idt[4], overflow_exception);
    SET_IDT_ENTRY(idt[5], bound_range_exceeded_exception);
    SET_IDT_ENTRY(idt[6], invalid_opcode_exception);
    SET_IDT_ENTRY(idt[7], device_not_available_exception);
    SET_IDT_ENTRY(idt[8], double_fault_exception);
    SET_IDT_ENTRY(idt[9], coprocessor_segment_overrun);
    SET_IDT_ENTRY(idt[10], invalid_TSS_exception);
    SET_IDT_ENTRY(idt[11], segment_not_present_exception);
    SET_IDT_ENTRY(idt[12], stack_fault_exception);
    SET_IDT_ENTRY(idt[13], general_protection_exception);
    SET_IDT_ENTRY(idt[14], page_fault_exception);
    SET_IDT_ENTRY(idt[16], x87_FPU_fp_error);
    SET_IDT_ENTRY(idt[17], alignment_check_exception);
    SET_IDT_ENTRY(idt[18], machine_check_exception);
    SET_IDT_ENTRY(idt[19], SIMD_fp_exception);
    SET_IDT_ENTRY(idt[128], sys_call_handler);

    SET_IDT_ENTRY(idt[33], asm_link_kb);   //kb interrupt
    SET_IDT_ENTRY(idt[40], asm_link_rtc);   //rtc interrupt
    SET_IDT_ENTRY(idt[32], asm_link_pit);   //pit interrupt
}
