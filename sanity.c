
#include "types.h"
#include "user.h"
#include "fcntl.h"

#define PAGE_SIZE 4096
#define H_SIZE 8
#define NUM_PAGES 13

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

void make_test(void (*f)(void) , int expected ,char * test_name){

    printf(1,"__________starting test %s_______________________\n",test_name);
    ans = 0;
    f();
    if(ans == expected)
        success++;
    else {
        fail++;
        printf(1,"%s failed!!\n",test_name);
    }

}


int main(void) {

    // __________________MALLOC___________________
    make_test(test_malloc, 2, "test_malloc");


    // _________________PMALLOC_PROTECT_PFREE_____________
    //  make_test(test_z, 0, "test_z");
    for (int i = 0; i < NUM_PAGES; i++){
        pnum = i;
        make_test(test_pmalloc_sanity, 1, "test_pmalloc_sanity");


    }
    for (int i = 0; i < NUM_PAGES; i++){
        pnum = i;
        make_test(test_pmalloc_pfree, 1, "test_pmalloc_pfree");
    }


    make_test(test_protected, 1, "test_protected");
    make_test(test_pfree_sanity, 2, "test_pfree_sanity");
    make_test(test_flag_clearing, 3, "test_flag_clearing");



    // ___________________SUMMERY_______________________________
    printf(1,"__________________SUMMERY________________________________\n");
    printf(1, "num of success:%d num of failures: %d\n", success, fail);

    if (fail == 0)
        printf(1, "All tests passed!! Yay!\n");
    exit();
}