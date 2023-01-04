#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "terminal.h"
#include "keyboard.h"
#include "rtc.h"
#include "filesys.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $8");
}

/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
// int idt_test(){
// 	TEST_HEADER;

// 	int i;
// 	int result = PASS;
// 	for (i = 0; i < 10; ++i){
// 		if ((idt[i].offset_15_00 == NULL) && 
// 			(idt[i].offset_31_16 == NULL)){
// 			assertion_failure();
// 			result = FAIL;
// 		}
// 	}

// 	return result;
// }

// // add more tests here

// int div_zero_test(){
// 	TEST_HEADER;
// 	int j;
// 	int i = 1;
// 	j = i/0;
// 	return FAIL;
// }

// int vidmap_test(){
// 	TEST_HEADER;
// 	int j;
// 	int i = 1;
// 	j = i/0;
// 	return FAIL;
// }

// int paging_test(){
// 	TEST_HEADER;
// 	int c;
// 	unsigned int i = 3;
// 	int* j = &i;
// 	c = *j;
// 	return PASS;
// }

// int paging_test_2(){
// 	TEST_HEADER;
// 	int i;
// 	int* j = NULL;
// 	i = *j;
// 	return FAIL;
// }

// int test_terminal(){
// 	TEST_HEADER;
// 	int nbytes;
// 	char buf[1024];
//     while(1){
// 		//terminal_write(0, (char*)"TESTING size 10\n", 16);
// 		nbytes = terminal_read(0, buf, 128);
//         terminal_write(0, buf, nbytes);
//     }
// 	// return FAIL;
// 	// char* buf = "test buffer";
// 	// terminal_write(1, buf, 11);
// 	return FAIL;
// }

// int test_rtc() {
// 	TEST_HEADER;
// 	uint32_t i;
// 	uint32_t j;
// 	int32_t returnVal = 0;
// 	returnVal += rtc_open(NULL);
// 	for(i = 2; i <= 1024; i *= 2){
// 		clear();
// 		clear_pos();
// 		returnVal += rtc_write(NULL, &i, sizeof(uint32_t));
// 		printf("%d Hz Test:\n", i);
// 		for(j = 0; j < i; j++){
// 			returnVal += rtc_read(NULL, NULL, NULL);
// 			printf("1");
// 		}
// 		printf("\n");
// 	}
// 	if(returnVal == 0){
// 		return PASS;
// 	}
// 	return FAIL;
// }

// /* Checkpoint 2 tests */



// int cat_test(uint8_t* filename){
// 	TEST_HEADER;
// 	int i;
// 	uint32_t file_len;
// 	dentry_t dentry;
// 	char buf[10000];
// 	clear();
// 	clear_pos();
// 	if(open_f(filename) == -1){
// 		printf("invalid filename");
// 		return FAIL;
// 	}
// 	if(read_dentry_by_name(filename, &dentry) == -1){
// 		return FAIL;
// 	}
// 	file_len = get_file_len(&dentry);
// 	if(read_data(dentry.inode_num, 0 , buf, file_len) == -1) {
// 		return FAIL;
// 	}
// 	clear();
// 	clear_pos();
// 	for(i = 0; i < file_len; i++){
// 		if(buf[i] == NULL){
// 			continue;
// 		}
// 		putc(buf[i]);
// 	}
// 	// clear();
// 	// clear_pos();
// 	// printf("%u", file_len);
// 	//i = 0;
// 	// while(buf[i]){
// 	// 	putc(buf[i]);
// 	// 	i++;
// 	// }
// 	return PASS;	
// }

// int cat_exec_test(uint8_t* filename){
// 	TEST_HEADER;
// 	int i;
// 	dentry_t dentry;
// 	char buf[10000];
// 	clear();
// 	clear_pos();
// 	if(open_f(filename) == -1){
// 		printf("invalid filename");
// 		return FAIL;
// 	}
// 	if(read_dentry_by_name(filename, &dentry) == -1){
// 		return FAIL;
// 	}
// 	if(read_data(dentry.inode_num, 0 , buf, 1000) == -1) {
// 		return FAIL;
// 	}
// 	clear();
// 	clear_pos();
// 	for(i = 0; i < 1000; i++){
// 		putc(buf[i]);
// 	}
// 	// clear();
// 	// clear_pos();
// 	// printf("%u", file_len);
// 	//i = 0;
// 	// while(buf[i]){
// 	// 	putc(buf[i]);
// 	// 	i++;
// 	// }
// 	return PASS;	
// }

// // int list_dir(){
// // 	TEST_HEADER;
// // 	char buf[1000];
// // 	int i;
// // 	int ret;
// // 	clear();
// // 	clear_pos();
// // 	for(i = 0; i < 62; i++){
// // 		ret = read_d(i, buf, 32);
// // 		if(ret == 0) break;
// // 		if(ret == -1) return FAIL;
// // 		printf(buf);
// // 		printf("\n");
// // 	}
// // 	return PASS;
// // }
// /* Checkpoint 3 tests */

// int sys_call_test(){
// 	#define DO_CALL(name,number)       
// 	asm volatile ("                    
// 	.GLOBL " #name "                  ;
// 	" #name ":                        ;
// 			PUSHL	%EBX              ;
// 		MOVL	$" #number ",%EAX ;
// 		MOVL	8(%ESP),%EBX      ;
// 		MOVL	12(%ESP),%ECX     ;
// 		MOVL	16(%ESP),%EDX     ;
// 		INT	$0x80             ;
// 		CMP	$0xFFFFC000,%EAX  ;
// 		JBE	1f                ;
// 		MOVL	$-1,%EAX	  ;
// 	1:	POPL	%EBX              ;
// 		RET                        
// 	")
// 	DO_CALL(write_handler, 4);
// }

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("page test", paging_test());
	//TEST_OUTPUT("page test 2", paging_test_2());
	//TEST_OUTPUT("div_zero_test", div_zero_test());
	// TEST_OUTPUT("file system cat test", cat_test("frame1.txt"));
	// TEST_OUTPUT("list dir test", list_dir());
	// //TEST_OUTPUT("term", test_terminal());
	//TEST_OUTPUT("rtc", test_rtc());
	//TEST_OUTPUT("sys call test", sys_call_test());
	while(1);
	// launch your tests here
	
}
