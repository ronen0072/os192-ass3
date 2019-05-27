#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

#define PAGE_SIZE 4096

typedef struct Header {
    struct Header *next;
    uint size;
} Header;

static Header end;
static Header *freep = &end;

static inline boolean isMeregeNeeded(Header *h) {
    uint size = h->size + sizeof(Header);
    uint freep_int = (uint)freep;
    uint h_int = (uint)h;
    return h_int+size == freep_int;
}

static inline void meregeFirst(Header *h) {
    uint size = freep->size + sizeof(Header);
    h->size += size;
    h->next = freep->next;
    freep = h;
}

static inline void pushFirst(Header * h) {
    if(freep != &end && isMeregeNeeded(h))
        meregeFirst(h);
    else {
        h->next = freep;
        freep = h;
    }
}

static inline boolean isMeregeFromLeftNeeded(Header* h, Header* cur) {
    uint size = cur->size + sizeof(Header);
    uint h_int = (uint)h;
    uint cur_int = (uint)cur;
    return h_int == cur_int + size;
}

static inline void meregeFromLeft(Header* h, Header* cur) {
    cur->size += h->size + sizeof(Header);
}

static inline boolean isMeregeFromRightNeeded(Header *h, Header *next) {
    if(next == &end)
        return false;

    uint size = h->size + sizeof(Header);
    uint next_int = (uint)next;
    uint h_int = (uint)h;
    return h_int+size == next_int;
}

static inline void meregeFromRight(Header *h, Header *next) {
    h->size += next->size + sizeof(Header);
    h->next = next->next;
}

static inline void insertBetween(Header *cur, Header* next, Header * h) {
    if(isMeregeFromLeftNeeded(h, cur)) { // if cur fits from left to h exactly
        meregeFromLeft(h, cur);
        if(isMeregeFromRightNeeded(cur, next)) //are merge and cur fitted for union??
            meregeFromRight(cur, next);
    } else { // if doesnt fit perfectly
        cur->next = h; // put h next to cur
        if(isMeregeFromRightNeeded(h, next)) // check if merge is neede in the right side of h
            meregeFromRight(h, next);
        else
            h->next = next;
    }
}

void free(void *arr) {
    Header* h = ((Header*)arr) - 1; // get the header
    uint h_int = (uint)h;

    if(freep == &end) { // if this is the first in the list initialize list with it
        h->next = freep;
        freep = h;
    } else { // it's not the first, insert it by adress order
        Header *cur = freep;
        int cur_int = (uint)cur;
        if(h_int < cur_int) // if it is the lowest so far
            pushFirst(h);
        else for(Header * p = freep->next; ; p = p->next)  { //find its place
                cur_int = (uint)cur;
                uint p_int = (uint)p;

                if(p == &end || (h_int > cur_int && h_int < p_int)) {
                    insertBetween(cur, p, h);
                    return;
                }
                cur = p;
            }
    }
}

static inline Header * split(Header * max, uint size) {
    char * max_char = (char *)max;
    max_char += (max->size - size); // find the splitting point/byte adress
    Header * ans = (Header *)max_char;
    ans->size = size;
    max->size -= (size + sizeof(Header)); // fix the size of the left part
    return ans;
}

void * malloc(uint size) {
    Header *max = freep;
    Header *max_perv = freep;
    Header *h_prev = freep;

    //find max
    for(Header *h = freep; h != &end; h = h->next) {
        if(max->size < h->size) {
            max = h;
            max_perv = h_prev;
        }
        h_prev = h;
    }

    Header *ans;
    if(max == &end || max->size < size) { // there is no space with wanted size
        char *sbrk_ans = sbrk(size + sizeof(Header)); //request space from kernel

        if(sbrk_ans == (char *)-1) //malloc failed
            return null;

        ans = (Header *)sbrk_ans;
        ans->size = size;
    } else if(max->size - size > sizeof(Header)) // need to split if its too big
        ans = split(max, size);
    else { // we have exactly what we need
        ans = max; //max->size >= size
        if(max == freep)
            freep = freep->next;
        else
            max_perv->next = max->next;
    }

    return ans + 1;
}

// sign that page was allocated with pmalloc
boolean init_pmalloc(void * page){
    if (/*reset_avl(page) == -1 || */sign_pa(page) == -1 )
        return false;
    return true;
}

void* pmalloc(){
//malloc a spare size (double+h+2)
    void * chunk= malloc(PAGE_SIZE*2+ sizeof(Header)*2);
    if(chunk == 0)
        return 0;
    Header* h = ((Header*)chunk) - 1; // get the header

    uint chunk_size= h->size;
    uint sizeOfHeader = sizeof(Header);
    //if adress is page-aligned split the chunk else make page page-aligned and then split.
    if((uint)chunk % PAGE_SIZE == 0) {
        h->size = PAGE_SIZE; //
        Header * rest = ((Header *) (chunk + PAGE_SIZE));
        rest->size = chunk_size - PAGE_SIZE - sizeOfHeader;
        rest->next=h->next;
        h->next=rest;
        void * rest_addr = chunk + PAGE_SIZE + sizeOfHeader;
        free(rest_addr);// return rest of the chunk back to stock
        if(init_pmalloc(chunk) == false){
            free(chunk);
            return 0;
        }
        return chunk;
    }
    else {
        uint remainder = (((uint) chunk) % PAGE_SIZE);
        uint alignment= PAGE_SIZE - remainder;
        if(alignment < sizeOfHeader) // i.e we don't have enough for the header
            alignment = alignment+PAGE_SIZE;
        void * ans =  chunk + alignment;
        Header* h_ans = ((Header*)ans) - 1;
        h_ans->size = PAGE_SIZE;
        h->size = alignment - sizeOfHeader;
        // arrange splitted chunks
        void * r_chunk = ans + h_ans->size + sizeOfHeader;
        Header * h_r_chunk =  ((Header*)r_chunk) - 1;
        h_r_chunk->size =  remainder + sizeOfHeader;

        if(alignment < sizeOfHeader){ // i.e we don't have enough for the header
            h->size = h->size + PAGE_SIZE;
            h_r_chunk->size = sizeOfHeader - remainder;
        }

        h_r_chunk->next = h->next;
        h->next = h_ans;
        h_ans->next = h_r_chunk;
        //return un-needed chunks to stock
        free(chunk);
        free(r_chunk);

        if(init_pmalloc(ans) == false){
            free(ans);
            return 0;
        }
        return ans;
    }
}

int protect_page(void* ap){

    if(get_pa_bit(ap) == 0){
        return -1;
    }
    //check that the pointer point to the start of the page
    if((uint) ap % PAGE_SIZE != 0 )
        return -1;
    protect_p(ap);
    return 1;

}

int pfree(void* ap){

    if(get_pa_bit(ap) == 0 /*|| get_w_bit(ap) == 1*/) // page is not pmalloced
        return -1;
    unprotect_p(ap);
    if((uint)ap % PAGE_SIZE != 0) // not starting point of page
        return -1;
   // reset_avl(ap);// reset avl bits;
    free(ap);
    return 1;


}

