/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <spl.h>
#include <spinlock.h>
#include <current.h>
#include <mips/tlb.h>
#include <addrspace.h>
#include <vm.h>
#include <proc.h>

/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 *
 * UNSW: If you use ASST3 config as required, then this file forms
 * part of the VM subsystem.
 *
 */

struct addrspace *
as_create(void)
{
	struct addrspace *as;

	as = kmalloc(sizeof(struct addrspace));
	if (as == NULL) {
		return NULL;
	}

	/*
	 * Initialize as needed.
	 */
	as->region_list = NULL;
	return as;
}

int
as_copy(struct addrspace *old, struct addrspace **ret)
{
	struct addrspace *newas;

	newas = as_create();
	if (newas==NULL) {
		return ENOMEM;
	}

	/*
	 * Write this.
	 */
	// region_list->next->next->next->NULL
	struct region *curr = old->region_list;
	while (curr != NULL) {
		int result = as_define_region(newas, curr->v_add, curr->size, curr->readable, curr->writable, true);
		if (result) {
			as_destroy(newas);
			return result;
		}
		curr = curr->next;
	}
	//////////////////TODO///////////////////
	// also copy on the hpt
	curr = old->region_list;
	while (curr != NULL) {
		int result = hpt_copy(curr->v_add, curr->size, curr->writable, old, newas);
		if (result) {
			as_destroy(newas);
			return result;
		}
		curr = curr->next;
	}

	*ret = newas;
	return 0;
}

void
as_destroy(struct addrspace *as)
{
	/*
	 * Clean up as needed.
	 */

	//////////////////TODO///////////////////
	//also clean hpt

	struct region *curr = as->region_list;
	struct region *prev;
	while (curr != NULL) {
		hpt_free(as, curr->v_add, curr->size);
		prev = curr;
		curr = curr->next;
		kfree(prev);
	}
	kfree(as);
}

void
as_activate(void)
{
	struct addrspace *as;

	as = proc_getas();
	if (as == NULL) {
		/*
		 * Kernel thread without an address space; leave the
		 * prior address space in place.
		 */
		return;
	}

	/*
	 * Write this.
	 */
	// FROM dumbvm.c
	int spl = splhigh();

	for (int i=0; i<NUM_TLB; i++) {
		tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
	}

	splx(spl);
}

void
as_deactivate(void)
{
	/*
	 * Write this. For many designs it won't need to actually do
	 * anything. See proc.c for an explanation of why it (might)
	 * be needed.
	 */
	as_activate();
}

/*
 * Set up a segment at virtual address VADDR of size MEMSIZE. The
 * segment in memory extends from VADDR up to (but not including)
 * VADDR+MEMSIZE.
 *
 * The READABLE, WRITEABLE, and EXECUTABLE flags are set if read,
 * write, or execute permission should be set on the segment. At the
 * moment, these are ignored. When you write the VM system, you may
 * want to implement them.
 */
int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t memsize,
		 int readable, int writeable, int executable)
{
	/*
	 * Write this.
	 */
	(void) executable;

    memsize += vaddr & ~(vaddr_t)PAGE_FRAME;
    vaddr &= PAGE_FRAME;
    /* ...and now the length. */
    memsize = (memsize + PAGE_SIZE - 1) & PAGE_FRAME;

	if ((vaddr + memsize) > MIPS_KSEG0) {
		return EFAULT;
	}

	// struct region *curr = as->region_list;

	// while (curr != NULL) {
	// 	if ((curr->v_add + curr->size) <= vaddr && (vaddr + memsize) >= curr->v_add) {
	// 		kprintf("pass check on kusg\n");
	// 		return EINVAL;
	// 	}
	// 	curr = curr->next;
	// }

	struct region *new = kmalloc(sizeof(struct region));
	if (new == NULL) {
		return ENOMEM;
	}

	new->v_add = vaddr;
	new->size = memsize;
	new->readable = readable;
	new->writable = writeable;
	new->old_rw = writeable;
	new->next = NULL;


	new->next = as->region_list;
	as->region_list = new;
	return 0;
}

int
as_prepare_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */
	// Make READONLY regoins
	if (as == NULL) return EFAULT;
	
	struct region *curr = as->region_list;

	while (curr != NULL) {
		curr->old_rw = curr->writable;
		curr->writable = true;
		curr = curr->next;
	}
	return 0;
}

int
as_complete_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */
	if (as == NULL) return EFAULT;	
	struct region *curr = as->region_list;
	while (curr != NULL) {
		curr->writable = curr->old_rw;
		curr = curr->next;
	}
	as_activate();
	return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	/*
	 * Write this.
	 */


	/* Initial user-level stack pointer */
	*stackptr = USERSTACK;
	return as_define_region(as, USERSTACK - PAGE_SIZE * 16, PAGE_SIZE * 16, true, true, false);
}

