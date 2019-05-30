#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"


void swap(struct page *xp, struct page *yp){
    struct page temp = *xp;
    *xp = *yp;
    *yp = temp;
}

// A function to implement bubble sort
void sort(struct pageArray* RAMpgs, int n){
    int i, j;
    for (i = 0; i < n-1; i++)

        // Last i elements are already in place
        for (j = 0; j < n-i-1; j++)
            if (RAMpgs->pages[j].ctime > RAMpgs->pages[j+1].ctime)
                swap(&(RAMpgs->pages[j]), &(RAMpgs->pages[j+1]));
}

// TODO -> check impl
void clearbit (uint* dest, uint mask){
    (*dest) = ((*dest) & (~mask));
    lcr3(V2P(myproc()->pgdir));
}

void addbit(uint* dest, uint mask){
    (*dest) = ((*dest) | (mask));
    lcr3(V2P(myproc()->pgdir));
}


// method to find free page in fdata structre of pages..
int findUnuesd(struct pageArray* metadataDB){
    for(int i = 0; i < MAX_PSYC_PAGES; ++i){
        if(metadataDB->pages[i].inUesd)
            continue;
        return i;
    }
    return -1;
}

int findVA(struct pageArray* DB, uint va){
    for(int i = 0; i < MAX_PSYC_PAGES; ++i){
        if(DB->pages[i].inUesd && DB->pages[i].va == va)
            return i;
    }
    return -1;
}


int FIFOchoose(struct proc* p){
    struct pageArray* RAMpgs = &(p->RAMpgs);
    struct pageArray copy    = *RAMpgs;
    int i = 0;

    sort(&copy, MAX_PSYC_PAGES);

    while(1){
        if(copy.pages[i].inUesd) {
            //free one
            walkpgdir(p->pgdir, (const void *) copy.pages[i].va, 0);

            //return answer
            return findVA(RAMpgs, copy.pages[i].va);

        }
        i = (i + 1) % MAX_PSYC_PAGES;

    }
}


int SCFIFOchoose(struct proc* p){
    struct pageArray* RAMpgs = &(p->RAMpgs);
    struct pageArray copy    = *RAMpgs;
    pte_t * pte;
    int i = 0;

    sort(&copy, MAX_PSYC_PAGES);

    while(1){
        if(copy.pages[i].inUesd) {
            //free one
            pte = walkpgdir(p->pgdir, (const void *) copy.pages[i].va, 0);

            if (!(*pte & PTE_A))
                //return answer
                return findVA(RAMpgs, copy.pages[i].va);
            clearbit(pte,PTE_A);
        }
        i = (i + 1) % MAX_PSYC_PAGES;
    }
}


int choosePageToSwapOut(struct proc* p){
    int indx = 0;
#ifdef FIFO
    indx = FIFOchoose(p);
#endif
#ifdef SCFIFO
    indx = SCFIFOchoose(p);
#endif
    p->pgout++;
    return indx;

}

//int swapOut(int indx, struct proc* p){
//  struct pageArray * RAMpgs  = &(p->RAMpgs);
//  struct pageArray * SWAPpgs = &(p->SWAPpgs);
//
//  if(SWAPpgs->size == 16)
//	  panic("SwapOut :: Swap File is full!!");
//
//  // getting file to swap out meta data
//  struct page tswap = RAMpgs->pages[indx];
//  insertSwaPpgs((char*)tswap.va, tswap.mem);
//
//  RAMpgs->pages[indx].inUesd = 0;
//  RAMpgs->size--;
//
//  return 1;
//}

int swapOut(int indx, struct proc* p, uint addr){
    struct pageArray * RAMpgs  = &(p->RAMpgs);
    struct pageArray * SWAPpgs = &(p->SWAPpgs);
    int sindx = findVA(SWAPpgs,addr);


    // getting file to swap out meta data
    uint va = RAMpgs->pages[indx].va;
//    void * mem = RAMpgs->pages[indx].mem;
    pte_t * pte_1 = walkpgdir(p->pgdir, (char*)va, 0);
    uint pa = *pte_1 & ~0xfff;
    void * mem = (void *)P2V(pa);

    RAMpgs->pages[indx].ctime = 0;
    RAMpgs->pages[indx].inUesd = 0;
    RAMpgs->size--;

    void * smem = kalloc();
    if(!smem){
        cprintf("swapIn:: Out of memory\n");
        return 0;
    }



    pte_t * pte = walkpgdir(p->pgdir, (char*)addr, 0);

    readFromSwapFile(p, smem, sindx*PGSIZE, PGSIZE);
    insertRAMPgs((char*)addr,smem);
    uint flags = *pte & 0xfff;

    *pte = V2P(smem)| flags | PTE_P ;
    *pte &= ~PTE_PG;


    SWAPpgs->pages[sindx].inUesd = 0;
    SWAPpgs->size--;

    insertSwaPpgs((char*)va,mem);
    kfree(mem);
    lcr3(V2P(myproc()->pgdir));
    return 1;
}

// 0-> error, 1-> success
int swapIn(uint va, struct proc* p){
    struct pageArray * RAMpgs  = &(p->RAMpgs);
    struct pageArray * SWAPpgs = &(p->SWAPpgs);
    char * mem;
    int OutIndx;

    // if too much pages
    if(RAMpgs->size == 16)
        panic("swapIn:: To Many Pages in RAM..");
    // find in swap file
    OutIndx = findVA(SWAPpgs, va);
    if(OutIndx == -1)
        panic("swapIn:: got to swapIn but not found in DB..");
    // find free page
    mem = kalloc();
    if(!mem){
        cprintf("swapIn:: Out of memory\n");
        return 0;
    }
    pte_t * pte = walkpgdir(p->pgdir, (char*)va, 0);
    //resetting changes
    readFromSwapFile(p, mem, OutIndx*PGSIZE, PGSIZE);

    insertRAMPgs((char*)va, mem);

    SWAPpgs->pages[OutIndx].inUesd = 0;
    SWAPpgs->size--;

    uint flags = *pte & 0xfff;
    *pte = V2P(mem)| flags | PTE_P ;
    *pte &= ~PTE_PG;
    lcr3(V2P(myproc()->pgdir));
    return 1;
}
void cleanPages(struct proc* p) {
    if (p->pid > DEFAULT_PROCESSES) {
        p->pgflt = 0;
        p->pgout = 0;
        p->pnum = 0;
        struct pageArray *RAMpgs = &(p->RAMpgs);
        struct pageArray *SWAPpgs = &(p->SWAPpgs);
        for (int i = 0; i < MAX_PSYC_PAGES; i++) {
            RAMpgs->pages[i].va = 0;
            RAMpgs->pages[i].mem = 0;
            RAMpgs->pages[i].ctime = 0;
            RAMpgs->pages[i].inUesd = 0;

            SWAPpgs->pages[i].va = 0;
            SWAPpgs->pages[i].mem = 0;
            SWAPpgs->pages[i].ctime = 0;
            SWAPpgs->pages[i].inUesd = 0;
        }
    }
}
