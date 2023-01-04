
#define ASM 1

.text

.global goto_user, flush_tlb
#goto_user(USER_DS, stack_pointer, eflag_bitmask, USER_CS, prog_eip);
goto_user:
    pushl %ebp
    movl %esp, %ebp
    movl 8(%ebp), %eax 
    pushl %eax  #pushing SS
    movl 12(%ebp), %ebx
    pushl %ebx  #pushing esp
    pushfl
    popl %ecx   #getting 32-bit eflags into ecx
    movl 16(%ebp), %edx
    orl %edx, %ecx #bit masking to allow interrupts
    pushl %ecx  #pushing eflags
    movl 20(%ebp), %edi
    pushl %edi  #pushing CS
    movl 24(%ebp), %esi
    pushl %esi  #pushing eip
    # addl $32, %esp
    # addl $32, %ebp
    iret    #switch to userspace

flush_tlb:
    movl %cr3, %eax #reloading the cr3 register
    movl %eax, %cr3
    ret








