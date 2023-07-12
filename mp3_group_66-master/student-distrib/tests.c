#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "terminal.h"
#include "FileSys.h"

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
	asm volatile("int $15");
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
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	return result;
}

/***
 * tests divide by 0 handler
 * INPUT: None
 * OUTPUT: Returns pass if passed
 * SIDE EFFECTS: None
 */

int div_zero_test()
{
	printf("testing div by 0\n");
	int x;
	int j = 0;
	x = 4/j;
	return PASS;
}

/***
 * tests null handler
 * INPUT: None
 * OUTPUT: Returns pass if passed
 * SIDE EFFECTS: None
 */
int NULL_test()
{
	printf("testing dereference NULL\n");
	int * x = NULL;
	int y = *x;
	y++;
	return PASS;
}



/***
 * tests paging memory bounds
 * INPUT: None
 * OUTPUT: Returns pass if passed
 * SIDE EFFECTS: None
 */
int paging_test1(){
	TEST_HEADER;
	printf("testing inbounds memory\n");
	int * x = (int*)0xB8000;

	x++;
	return PASS;
}

/***
 * tests paging memory bounds
 * INPUT: None
 * OUTPUT: Returns pass if passed
 * SIDE EFFECTS: None
 */
int paging_test2(){
	TEST_HEADER;
	int * x;

	printf("Testing accessing outside present memory\n");
	x = (int*)0xB7fff;
	x++;
	return PASS;
}

/***
 * tests paging dereference null
 * INPUT: None
 * OUTPUT: Returns pass if passed
 * SIDE EFFECTS: None
 */
int paging_test3(){
	TEST_HEADER;

	int * x;
	printf("testing dereference NULL\n");
	x = NULL;
	int y = *x;
	y++;
	return PASS;
}
/*
//tests terminal write, check terminal for terminal output
int write_terminal_test(){
	uint32_t message[12] = "hello \n bye ";
	int8_t i;
	i = 2;
	open_terminal((const uint8_t *)&i);
	write_terminal((int32_t)1, message, (int32_t)12);
	return PASS;
}
*/
/*echos what you type in the terminal on the next line */
int read_terminal_test(){
	int j;
	int8_t i;
	i = 2;
	uint8_t buffer[T_BUF_SIZE];
	open_terminal((const uint8_t *)&i);
	read_terminal(0,buffer,T_BUF_SIZE);
	
	for(j = 0; j < T_BUF_SIZE;j++){
		putc(buffer[j]);
	}
	putc('\n');
	close_terminal(0);
	return PASS;
}



/* Checkpoint 2 tests */

//rtc test cases for determine rate
//input: none
//output: (return pass anyways) user should quit when done
// side effects: changes rate of rtc.  Know if pass by speed of prints change
int rtc_test(){

	//-----------------
	// UNCOMMENT RTC.C LINE 68-79
	//-----------------

	//run through many frequencies
	int j;
	int rtc_ctr = 0;

	//rtc_write(1, &frequency_test, 4); //sets frequency to whatever you want
	//test loop
	while(1){
		clear();
		for(j=2;j<1024;j*=2){
		rtc_write(1, &j, 4); //sets frequency to whatever you want
		set_rtc_counter(0);
		do{
			rtc_ctr = get_rtc_counter(); 
		}while(rtc_ctr < j*2);

		}
	}

	return PASS;
	
	//rtc_frequency_change(2);
}



/***
 * tests reading a directory
 * INPUT: None
 * OUTPUT: none
 * RETURN: file descriptor
 * SIDE EFFECTS: modifies PCB, prints to screen
 */
int32_t dir_read_test()
{
	printf("Testing dir_read \n");

	int32_t fd, cnt;
    uint8_t buf[33];	//one more than the max dir name length

    if (-1 == (fd = dir_open ((uint8_t*)"."))) {	//attempt to open directory
        printf("directory open failed\n");
        return -1;
    }

    while (0 != (cnt = dir_read (fd, buf, 32))) {	//read all files in dir, until fail or EOD
        if (-1 == cnt) {
	        printf("directory entry read failed\n");
	        return fd;
	    }
		else if (-2==cnt)
		{
			printf("Printed all dentries\n");
	        return fd;
		}
	    printf("%s\n", buf);
    }

    return fd;	

}


/***
 * tests reading a file
 * INPUT: None
 * OUTPUT: none
 * RETURN: file descriptor
 * SIDE EFFECTS: modifies PCB, prints to screen
 */
int32_t file_read_test(const uint8_t* filename)
{
		printf("Testing file_read \n");

	int32_t fd;				
	int32_t ec=512;			//error code for read, set to 512 to enter loop
    uint8_t buf[1024];		//oversized buffer for reads
    if (-1 == (fd = file_open (filename))) { //attempt to open file
        printf("File open failed\n");
        return -1;
    }
	else
		printf("open success on fd = %d\n", fd);


	printf("\nFile contents:\n\n");

	while(ec == 512)					//loop thru file until EOF or error in groups of 512 bytes
	{
		ec = file_read(fd, buf, 512);

		//printf("return code: %d\n", ec);
		if (-1 == ec) {
			printf("File entry read failed -1\n");
			return fd;
		}
		if ( -2 == ec) {
			printf("File entry read overflow -2\n");
			return fd;
		}
		printf("%s", buf);
	}
    return fd;	

}





/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/***
 * Function to test every (comment out and on if using)
 * INPUT: None
 * OUTPUT: None
 * SIDE EFFECTS: Runs tests
 */
void launch_tests(){
	// while(1){
	read_terminal_test();
	// }
	//write_terminal_test();

	//CP1 Tests
	//TEST_OUTPUT("idt_test", idt_test());
	//NULL_test();
	//paging_test1();
	//paging_test2();
	//paging_test3();
	//TEST_OUTPUT("terminal write test", write_terminal_test());
	//TEST_OUTPUT("terminal read test - newline", read_terminal_test());
    //TEST_OUTPUT("paging_test", paging_test());
	//TEST_OUTPUT("paging_test", paging_test());
	
	//CP2 Tests
	//int32_t fd[5];
	//rtc_test();

	/*
	fd[0] = dir_read_test();												//immitate ls
	dir_close(fd[0]);														//close dir
	fd[1] = file_read_test((uint8_t*)"frame0.txt");							//small text
	fd[2] = file_read_test((uint8_t*)"frame1.txt");							//small text and close file
	file_close(fd[2]);
	fd[3] = file_read_test((uint8_t*)"verylargetextwithverylongname.txt");	//large text
	
	fd[4] = file_read_test((uint8_t*)"ls");									//exe
	
	printf("\n \nFD: %d %d %d %d %d \n", fd[0],fd[1],fd[2],fd[3],fd[4]);
	printf("end tests\n");
	*/
	//int32_t trash=file_read_test((uint8_t*)"asdf.txt");	(void)trash;	//test absent file
	//int32_t trash2=file_read_test((uint8_t*)NULL);		(void)trash2;	//test NULL file
	return;

}

