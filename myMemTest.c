#include "types.h"
#include "user.h"


// TESTS FILE
#define PGSIZE		4096

#define PAGE_SIZE 4096
#define H_SIZE 8
#define NUM_PAGES 13
//int success=0, fail=0,ans=0;

int testnum = 20;
int success=0, fail=0,ans=0, fibNum=10,mid=-1;
void * pages[NUM_PAGES] ; int pnum=0;
int loopnum=1;

int num_threads=1;

int fib(uint num){
    if (num == 0)
        return 0;
    if(num == 1)
        return 1;
    return fib(num-1) + fib(num-2);
}

//sanity to check that replaced malloc works properly
void test_malloc(){
    void * addr1 = malloc(500);
    if(addr1 == null) //malloc failed
        return;
    free(addr1);

    void * addr2 = malloc(500);
    if(addr2 == 0 )
        return;
    if((uint)addr1 == (uint) addr2){
        ans++;
    }
    else printf(1,"adrr1:%d, addr2%d\n",(uint)addr1, (uint)addr2);


    free(addr2);
    addr1 = malloc(300);
    if(addr1 == null) //malloc failed
        return;

    free(addr1);
    addr2 = malloc(300);
    if(addr2 == 0 )
        return;
    if((uint)addr1 == (uint) addr2){
        ans++;
    }
    else printf(1,"adrr1:%d, addr2%d\n",(uint)addr1, (uint)addr2);


}

void test_pmalloc_sanity(){
    pages[pnum] = pmalloc();
    if(pages[pnum] == 0){
        printf(1,"pmalloc failed");
        return;
    }
    //test page aligned
    if((uint)pages[pnum] % 4096 == 0 )
        ans++;


}
void  test_pmalloc_pfree(){
    if(pfree(pages[pnum]) == 1);
    ans++;
}

void test_protected(){
    void * page = pmalloc();
    if(page == 0){
        printf(1,"pmalloc failed\n");
        return;
    }


    if(protect_page(page) == -1){
        printf(1,"page not pmalloced\n");
        ans=-1;
        return;
    }

    if(get_w_bit(page)== 0)
        ans++;

// try to memset
    //    memset(page,0,500);


}
void test_z(){
    uint sz = (uint) sbrk(0);
    printf(1, "proc size %d\n", (sz)/PAGE_SIZE+ (sz%PAGE_SIZE !=0));
    sbrk(18*4096-sz);
    sz = (uint) sbrk(0);
    printf(1, "proc size %d\n", (sz)/PAGE_SIZE+ (sz%PAGE_SIZE !=0));
}
void test_pfree_sanity(){
    void * page = pmalloc();
    void * mem = malloc(4096);

    if(page == 0){
        printf(1,"pmalloc failed\n");
        return;
    }
//    if(pfree(page)== -1){
//        ans++;
//    }

    if(protect_page(page) == -1){
        printf(1,"page not protected\n");
        ans=-1;
        return;
    }

    if(pfree(page) == 1) // free pmalooced
        ans++;

    if(mem!=0 && pfree(mem) == -1){ //doesnt free regular malloc
        ans++;
    }

}

void test_flag_clearing(){
    void * page = pmalloc();


    if(page == 0){
        printf(1,"pmalloc failed\n");
        return;
    }
//    if(pfree(page)== -1){
//        ans++;
//    }

    if(protect_page(page) == -1){
        printf(1,"page not protected\n");
        ans=-1;
        return;
    }

    if(pfree(page) == 1) // free pmalloced
        ans++;

    if(pfree(page) == -1) // should free cause its not"pmalloced" anymore
        ans++;

    void * mem = malloc(PAGE_SIZE); // malloc should use this
    if(mem!=0 && pfree(mem) == -1){ //doesnt free regular malloc
        ans++;
    }

}
int randomGenrator(int seed, int limit){
	return (seed ^ (seed * 7)) % limit;
}


void test2(void){
	char * bla = sbrk(20 * PGSIZE);
	
	for(int i = 0; i < 10; ++i){
		bla[i] = i;
		bla[20 - i] = i;
	}
	
	//reading
	for(int i = 0; i < 10; ++i){
		if(bla[i] != i || bla[20-i] != i){
			printf(1, "failed\n");
			ans = -1;
		}
	}
}

void test1(int flag){
	// request 20 pages.
	char * bla = sbrk(20*PGSIZE);
	
	// write i to page i
	for(int i = 0; i < 20; i++)
		bla[i*PGSIZE] = i;
	
	//read
	for(int i = 0; i < 20 ; i++){
		if(bla[i*PGSIZE] != i){
			printf(1, "Simple Test failed");
            ans = -1;
		}
	}
	if (flag) exit();
}

void mySimpleTets(void){
	int pid;
	
	if((pid = fork()) == -1) {
		printf(1, "fork failed exiting...\n");
        ans = -1;
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

void doubleProcess(){
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
void segTest(){
	int pid = fork();
	int* seg = 0;
	
	if(!pid){
		*seg = 1;
		printf(1,"failed\n");
        ans = -1;
	}
	wait();
	printf(1,"done\n");
}

void test3(void){
	sbrk(10*PGSIZE);
	exit();
}

void forktests(void){
    ans = -1;
	for (int i = 0; i < 5 ; i++){
		int pid = fork();
		if(pid)
			continue;
		test3();
	}
	for(int i = 0 ; i < 5; i++){
		wait();
	}
    ans = 1;
	printf(1,"done\n");
}

void multiplewritesOneProcess(void){
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

void overLoadPage(void){

	int pid = fork();
	if(!pid)
		sbrk(33*PGSIZE);
	wait();
	printf(1,"done\n");
}

void memtest(char* brk){

	for(int i = 0; i < 10; i++){
		for(int j = 0; j < 20; j++){
			if(brk[PGSIZE*i + j] == j )
				continue;
			printf(1, "mem test failed\n");
            ans = -1;
	    }
	exit();
    }
}

void copyMemTest(void){
	char* brk = sbrk(10*PGSIZE);
	
	for(int i = 0; i < 10; i++){
		for(int j = 0; j < 20; j++){
			brk[PGSIZE*i + j] = j;
		}
	}
	
	int pid = fork();
	if(!pid)
		memtest(brk);
	wait();
	printf(1, "done\n");
}

void make_test(void (*f)(void) , int expected ,char * test_name){

    printf(1,"_______________________starting test %s______________________\n",test_name);
    ans = 0;
    f();
    if(ans == expected)
        success++;
    else {
        fail++;
        printf(1,"%s failed!!\n",test_name);
    }

}
int main(){


    // __________________MALLOC___________________
   // make_test(test_malloc, 2, "test_malloc");


    // _________________PMALLOC_PROTECT_PFREE_____________
    //  make_test(test_z, 0, "test_z");
//    for (int i = 0; i < NUM_PAGES; i++){
//        pnum = i;
//        make_test(test_pmalloc_sanity, 1, "test_pmalloc_sanity");
//
//
//    }
//    for (int i = 0; i < NUM_PAGES; i++){
//        pnum = i;
//        make_test(test_pmalloc_pfree, 1, "test_pmalloc_pfree");
//    }
//
//
//    make_test(test_protected, 1, "test_protected");
//    make_test(test_pfree_sanity, 2, "test_pfree_sanity");
//    make_test(test_flag_clearing, 3, "test_flag_clearing");

	//first-> allocate 20 pages, write in lineric, meaning write each
	// page i the number i, and read it.
	//Sanity Test...

	make_test(mySimpleTets, 0,"mySimpleTets");
    make_test(doubleProcess, 0,"doubleProcess");
    make_test(segTest, 0,"segTest");
    make_test(forktests, 1,"forktests");
    //make_test(multiplewritesOneProcess, 0,"multiplewritesOneProcess");
    make_test(overLoadPage, 0,"overLoadPage");
    make_test(copyMemTest, 0,"copyMemTest");

    // ___________________SUMMERY_______________________________
    printf(1,"__________________SUMMERY________________________________\n");
    printf(1, "num of success:%d num of failures: %d\n", success, fail);

    if (fail == 0)
        printf(1, "All tests passed!! Yay!\n");
    exit();
}