
#define ASM 1

.text

.globl loadPageDirectory, enablePaging, flush

#loads the cr3 register to load the page directory
loadPageDirectory:
    pushl %ebp
    movl %esp, %ebp
    movl 8(%esp), %eax
    movl %eax, %cr3
    movl %ebp, %esp
    popl %ebp
    ret

#loads the cr4 register and cr0 register to enable paging for the 4MB page and the 4kB page
enablePaging:
    pushl %ebp
    movl %esp, %ebp
    movl %cr4, %eax
    orl $0x00000010, %eax
    movl %eax, %cr4
    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0
    movl %ebp, %esp
    popl %ebp
    ret
