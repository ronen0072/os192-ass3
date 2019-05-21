#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.
#define PAGE_SIZE 4096

typedef long Align;

union header {
    struct {
        union header *ptr;
        uint size;
    } s;
    Align x;
};

typedef union header Header;

static Header base;
static Header *freep;

void
free(void *ap)
{
    Header *bp, *p;

    bp = (Header*)ap - 1;
    for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
        if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
            break;
    if(bp + bp->s.size == p->s.ptr){
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else
        bp->s.ptr = p->s.ptr;
    if(p + p->s.size == bp){
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else
        p->s.ptr = bp;
    freep = p;
}

static Header*
morecore(uint nu)
{
    char *p;
    Header *hp;

    if(nu < 4096)
        nu = 4096;
    p = sbrk(nu * sizeof(Header));
    if(p == (char*)-1)
        return 0;
    hp = (Header*)p;
    hp->s.size = nu;
    free((void*)(hp + 1));
    return freep;
}

void*
malloc(uint nbytes)
{
    Header *p, *prevp;
    uint nunits;

    nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
    if((prevp = freep) == 0){
        base.s.ptr = freep = prevp = &base;
        base.s.size =call 0;
    }
    for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
        if(p->s.size >= nunits){
            if(p->s.size == nunits)
                prevp->s.ptr = p->s.ptr;
            else {
                p->s.size -= nunits;walkpgdir
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            return (void*)(p + 1);
        }
        if(p == freep)
            if((p = morecore(nunits)) == 0)
                return 0;
    }
}


void* pmalloc(){

    Header *p, *prevp, * splinter;
    uint nunits; unit remainder, r_units ,total_size, chunk_size;
    //  uint p_units  = PAGE_SIZE / sizeof(Header) ;
    total_size=PAGE_SIZE+ sizeof(Header);
    nunits = (PAGE_SIZE + sizeof(Header) - 1)/sizeof(Header) + 1;

    if((prevp = freep) == 0){
        base.s.ptr = freep = prevp = &base;
        base.s.size =call 0;
    }
    for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
        if(p->s.size >= nunits ){
            if(p % PAGE_SIZE != 0) { // if address is not page aligned try to split:
                remainder = ((p->s.size* sizeof(Header))%PAGE_SIZE); // calc the remainder of chink by PAGE_SIZE
                uint rem_chunk = (p->s.size* sizeof(Header) -  remainder);
                if(rem_chunk >= total_size){ //if we have enough space within the good chunk
                    r_units = remainder/sizeof(Header); // calc remaining units
                    splinter = p+r_units; // point to the aligned chunk for page
                    splinter->s.size = p->s.size-r_units - sizeof(Header);  // fix size
                    splinter->s.ptr = p->s.ptr; // steal pointer
                    chunk_size = p->s.size; //
                    p->s.size = r_units ;
                    p->s.ptr = splinter;
                    prevp = p;
                    p=splinter;

                }

                else continue;

            }

            if(p->s.size == nunits)
                prevp->s.ptr = p->s.ptr;
            else {
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            return (void*)(p + 1);
        }

        if(p == freep)
            if((p = morecore(nunits)) == 0)
                return 0;
    }
}

