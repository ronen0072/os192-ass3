#include "types.h"
#include "user.h"


// TESTS FILE
#define PGSIZE		4096

int
randomGenrator(int seed, int limit)
{
	return (seed ^ (seed * 7)) % limit;
}


void
test2(void)
{
	char * bla = sbrk(20 * PGSIZE);
	int i;
	
	for(i = 0; i < 10; ++i){
		bla[i] = i;
		bla[20 - i] = i;
	}
	
	//reading
	for(i = 0; i < 10; ++i){
		if(bla[i] != i || bla[20-i] != i)
			printf(1, "failed\n");
	}
}

void
test1(int flag)
{
	int i;
	// request 20 pages.
	char * bla = sbrk(20*PGSIZE);
	
	// write i to page i
	for(i = 0; i < 20; i++)
		bla[i*PGSIZE] = i;
	
	//read
	for(i = 0; i < 20 ; i++){
		if(bla[i*PGSIZE] != i){
			printf(1, "Simple Test failed");
		}
	}
	if (flag) exit();
}

void
mySimpleTets(void)
{
	int pid;
	printf(1, "mySimpleTest ------> ");
	
	if((pid = fork()) == -1) {
		printf(1, "fork failed exiting...\n");
		goto bad;
	}
	
	if(pid == 0)
		test1(1);
	else{
		wait();
		printf(1, "done\n");
	}
	return;
	
bad:
	exit();
}

void
doubleProcess()
{
	printf(1, "doubleProcess------> ");
	int pid = fork();
	if(!pid){
		test2();
		exit();
	}
	int pid2 = fork();
	if(!pid2){
		test2();
		exit();
	}
	wait();
	wait();
	printf(1,"done\n");
}

//checking segemntion fault
void
segTest()
{
	printf(1, "segTest-------> ");
	int pid = fork();
	int* seg = 0;
	
	if(!pid){
		*seg = 1;
		printf(1,"failed\n");
	}
	wait();
	printf(1,"done\n");
}

void
test3(void)
{
	sbrk(10*PGSIZE);
	exit();
}

void
forktests(void)
{
	printf(1, "forktest------> ");
	for (int i = 0; i < 5 ; i++){
		int pid = fork();
		if(pid)
			continue;
		test3();
	}
	for(int i = 0 ; i < 5; i++){
		wait();
	}
	printf(1,"done\n");
}

void
multiplewritesOneProcess(void)
{
	printf(1,"multiplewritesOneProcess\n");
	int pid = fork();
	if(pid)
		goto waitf;
	
	char buf[100];
	char* brk = sbrk(20*PGSIZE);
	printf(1, "choose seed for random genrator: ");
	gets(buf, 100);
	int random = randomGenrator(atoi(buf),20);
	
	for(int i = 0; i < PGSIZE; i++)
	{
		brk[random*i + i] = i;
		random = randomGenrator(random, 20);
	}
	
	printf(1, "done\n");
	exit();
	
waitf:
	wait();
}

void
overLoadPage(void)
{
	printf(1,"page over load ---->\n");
	int pid = fork();
	if(!pid)
		sbrk(33*PGSIZE);
	wait();
	printf(1,"done\n");
}

void
memtest(char* brk)
{
	int i,j;
	
	for(i = 0; i < 10; i++){
		for(j = 0; j < 20; j++){
			if(brk[PGSIZE*i + j] == j )
				continue;
			printf(1, "mem test failed\n");
	}
	exit();
}
}

void
copyMemTest(void)
{
	printf(1, "Copy Mem Test----> ");
	char* brk = sbrk(10*PGSIZE);
	int i, j;
	
	for(i = 0; i < 10; i++){
		for(j = 0; j < 20; j++){
			brk[PGSIZE*i + j] = j;
		}
	}
	
	int pid = fork();
	if(!pid)
		memtest(brk);
	wait();
	printf(1, "done\n");
}

int 
main()
{
	//first-> allocate 20 pages, write in lineric, meaning write each
	// page i the number i, and read it.
	//Sanity Test...
	//mySimpleTets();
	doubleProcess();
	segTest();
	forktests();
	multiplewritesOneProcess();
	overLoadPage();
	copyMemTest();
	
	exit();
}