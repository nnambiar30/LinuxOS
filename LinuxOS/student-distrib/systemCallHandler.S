
#define ASM 1

.text

.global sys_call_handler, ret_to_exec

.align 4
sys_call_table: #jump table holding the system calls
    .long 0
    .long sys_halt
    .long sys_execute
    .long sys_read
    .long sys_write
    .long sys_open
    .long sys_close
    .long getargs
    .long vidmap

.align 4
sys_call_handler:
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx  #saving the registers
    
    pushfl  #saving the flags

    pushl %edx
    pushl %ecx
    pushl %ebx  #pushing the arguments to the stack

    cmpl $11, %eax
    jae invalid_call
    cmpl $0, %eax
    jng invalid_call    #making sure the system call invoked is a valid call
    
    call *sys_call_table(, %eax, 4) #jumping the the correct system call
    jmp sys_call_complete

invalid_call:
    movl $-1, %eax

sys_call_complete:
    addl $12, %esp  #resetting stack pointer

    popfl
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp   #restoring flags and registers

    iret    #return to user

# .align 4
# ret_to_exec:
#     pushl %ebp
#     movl %esp, %ebp
#     andl $0, %ebx
#     movl 16(%ebp), %ebx
#     movl %ebx, %eax #restoring status
#     movl 12(%ebp), %ebx
#     movl %ebx, %esp #restoring esp
#     movl 8(%ebp), %ebx
#     movl %ebx, %ebp #restoring ebp
#     leave
#     ret #return to execute return
