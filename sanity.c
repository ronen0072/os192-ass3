
#include "types.h"
#include "user.h"
#include "fcntl.h"



int testnum = 20;
int success=0, fail=0,ans=0, fibNum=10,mid=-1;

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
    void * page = pmalloc();
    if(page == 0){
        printf(1,"pmalloc failed");
        return;
    }
    //test page aligned
    if((uint)page % 4096 == 0 )
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




int main(void){

    // __________________MALLOC___________________
    make_test(test_malloc, 2, "test_malloc");
    make_test(test_pmalloc_sanity, 1, "test_pmalloc_sanity");
    make_test(test_protected, 1, "test_protected"); //should create a trap


    // ___________________SUMMERY_______________________________
    printf(1,"__________________SUMMERY________________________________\n");
    printf(1, "num of success:%d num of failures: %d\n", success, fail);

    if (fail == 0)
        printf(1, "All tests passed!! Yay!\n");
    exit();
}