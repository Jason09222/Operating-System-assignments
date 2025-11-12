#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <addrspace.h>
#include <vm.h>
#include <machine/tlb.h>
#include <synch.h>

/* Place your page table functions here */

paddr_t hpt_size;

struct lock *hpt_locker;

void vm_bootstrap(void)
{
    /* Initialise any global components of your VM sub-system here.  
     *  
     * You may or may not need to add anything here depending what's
     * provided or required by the assignment spec.
     */
    
    init_hash_pagetable();
    hpt_locker = lock_create("hpt_locker");
}

int
vm_fault(int faulttype, vaddr_t faultaddress)
{
	struct addrspace *as;
	int spl;

	switch (faulttype) {
	    case VM_FAULT_READONLY:
            return EFAULT;
	    case VM_FAULT_READ:
	    case VM_FAULT_WRITE:
		    break;
	    default:
		    return EINVAL;
	}

	if (curproc == NULL || !hash_pagetable) {
		/*
		 * No process. This is probably a kernel fault early
		 * in boot. Return EFAULT so as to panic instead of
		 * getting into an infinite faulting loop.
		 */
		return EFAULT;
	}

	as = proc_getas();
	if (as == NULL) {
		/*
		 * No address space set up. This is probably also a
		 * kernel fault early in boot.
		 */
		return EFAULT;
	}

    uint32_t index = hpt_index(as, faultaddress);

    if (index == hpt_size) {
        return ENOMEM;
    }


    struct region *curr = as->region_list;
    struct page_entry *curr_pe = hash_pagetable[index];
    lock_acquire(hpt_locker);
    //lock_acquire(as->hpt_locker);
    // find internal chaining 
    while (curr_pe != NULL) {
        if (curr_pe-> pid == (paddr_t)as &&  curr_pe-> vpn == (faultaddress & PAGE_FRAME) && (curr_pe->elo & TLBLO_VALID) ){
            break;
        }
        curr_pe = curr_pe->next;
    }
    
    //check valid translation
    if (curr_pe == NULL /*|| (curr_pe-> pid != (paddr_t)as &&  curr_pe-> vpn != (faultaddress & PAGE_FRAME))*/) {
        //panic("invalid translation");
        //check valid region
        if (hash_pagetable[index] != NULL) {
            panic("invalid translation");
        }
        
        if (as->region_list == NULL) {
            lock_release(hpt_locker);
            return EFAULT;
        }
        
        while (curr != NULL) {
            if (faultaddress >= curr->v_add && (faultaddress - curr->v_add) <= curr->size) {

                break;
            }
            curr = curr->next;
        }
    
        if (curr == NULL) {
            lock_release(hpt_locker);

            return EFAULT;
        }

        int result = hpt_insert(as, faultaddress, curr->writable);
        if (result) {
            lock_release(hpt_locker);
            return result;
        }

    } 
    spl = splhigh();
    uint32_t ehi = faultaddress & PAGE_FRAME;
    struct page_entry *position = hpt_lookup(as, faultaddress);
    
    if (position == NULL) {
        panic("should not happen");
    }

    uint32_t elo = position->elo;
    tlb_random(ehi, elo);
    splx(spl);
    lock_release(hpt_locker);
    return 0;
}

/*
 * SMP-specific functions.  Unused in our UNSW configuration.
 */

void
vm_tlbshootdown(const struct tlbshootdown *ts)
{
	(void)ts;
	panic("vm tried to do tlb shootdown?!\n");
}


void init_hash_pagetable(void) {

    paddr_t num_page = ram_getsize() / PAGE_SIZE;
    
    hpt_size = num_page * 2;

    hash_pagetable = (struct page_entry **)kmalloc(hpt_size * sizeof(struct page_entry *));

    uint32_t i = 0;
    while(i < hpt_size){
        hash_pagetable[i] = NULL;
        i++;
    }

}

uint32_t hpt_index(struct addrspace *add_ptr, vaddr_t faultaddr) {
    
    uint32_t hash_index;
    hash_index = (((uint32_t)add_ptr) ^ (faultaddr >> 12)) % hpt_size;
    return hash_index;

}

void hpt_free(struct addrspace *as, vaddr_t add, size_t memsize) {
    
    vaddr_t curr_add = add;
    lock_acquire(hpt_locker);
    while(curr_add < add + memsize){
        uint32_t index = hpt_index(as, curr_add);
        
        struct page_entry *curr_entry = hash_pagetable[index];
        struct page_entry *prev = NULL;

        while (curr_entry != NULL) {
            
            if (curr_entry->pid == (paddr_t)as && curr_entry->vpn == (curr_add & PAGE_FRAME) && curr_entry->elo != 0) {
                
                if (prev != NULL) {
                    prev->next = curr_entry->next;
                }else {
                    hash_pagetable[index] = curr_entry->next;
                }
                //panic("current index %d, %d", old_index, curr_entry->elo);
                //panic("here %0x", curr_entry->elo);
                free_kpages(curr_entry->elo & TLBLO_PPAGE);
                
                break;

            }
            prev = curr_entry;
            curr_entry = curr_entry->next;
        }

        curr_add+= PAGE_SIZE;
    }
    lock_release(hpt_locker);
}

int hpt_insert(struct addrspace *as, vaddr_t v_add, bool writeble) {
     //allocate frame and zero fill
        vaddr_t frame = alloc_kpages(1);
        if(frame == 0) {
            lock_release(hpt_locker);
            return ENOMEM;
        }
        bzero((void *)frame, PAGE_SIZE);
        
        paddr_t pframe = KVADDR_TO_PADDR(frame) & PAGE_FRAME;
        //create new entry
        struct page_entry *newentry = (struct page_entry *)kmalloc(sizeof(struct page_entry));

        newentry->vpn = (v_add & PAGE_FRAME);

        newentry->pid = (uint32_t)as;

        newentry->elo = pframe | 0 |TLBLO_VALID;

        newentry->next = NULL;
        
        if (newentry == NULL) {
            return ENOMEM;
        }
        //check permission
        if (writeble) {
            newentry->elo |= TLBLO_DIRTY;
        }

        uint32_t index = hpt_index(as, v_add);

        if (hash_pagetable[index] != NULL) {
            struct page_entry *last = hash_pagetable[index];
            while(last->next != NULL) {
                last = last->next;
            }
            last->next = newentry;
        }else {
            hash_pagetable[index] = newentry;
            //panic("here %0x, %0x, %0x,",hash_pagetable[index]->pid, hash_pagetable[index]->vpn, hash_pagetable[index]->elo);
        }
        return 0;

 }

int hpt_copy(vaddr_t add, size_t memsize, bool writeable, struct addrspace *old_add, struct addrspace *new_add) {
    
    vaddr_t curr_add = add;
    if (curr_add == 0){
        return ENOMEM;
    }
    lock_acquire(hpt_locker);

    while(curr_add < add + memsize){
        uint32_t old_index = hpt_index(old_add, curr_add);
        struct page_entry *old_entry = hash_pagetable[old_index];
        // find internall channing for old address space
        while(old_entry != NULL){
            if (old_entry->pid == (paddr_t)old_add && old_entry->vpn == (curr_add & PAGE_FRAME)) {
                break;
            }
            old_entry = old_entry->next;
        }
        // if region exists on hpt 
        if (old_entry != NULL) {
            int result = hpt_insert(new_add, curr_add, writeable);
            if (result) {
                lock_release(hpt_locker);
                return ENOMEM;
            }
        }   
    
        curr_add += PAGE_SIZE;
    }
    lock_release(hpt_locker);
    return 0;
}

struct page_entry *hpt_lookup(struct addrspace *as, vaddr_t vaddr){
    uint32_t index = hpt_index(as, vaddr);
    if (hash_pagetable[index] == NULL) {
        return NULL;
    }
    struct page_entry *curr = hash_pagetable[index];
    while (curr != NULL) {
        if (curr->pid == (uint32_t)as && curr->vpn == (vaddr & PAGE_FRAME)){
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}