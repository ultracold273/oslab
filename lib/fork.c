// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	if (!(err & FEC_WR)) {
		panic("Page fault not on write.");
	}
	if (!(uvpt[PGNUM(addr)] & PTE_COW)) {
		panic("page fault not on copy-on-write.");
	}

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	r = sys_page_alloc(0, (void *)PFTEMP, PTE_W | PTE_P | PTE_U);
	if (r < 0) {
		panic("page alloc failed");
	}
	memmove((void *)PFTEMP, (void *)ROUNDDOWN(addr, PGSIZE), PGSIZE);
	r = sys_page_map(0, (void *)PFTEMP, 0, (void *)ROUNDDOWN(addr, PGSIZE), PTE_W | PTE_P | PTE_U);
	if (r < 0) {
		panic("page map failed");
	}

	//panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	//panic("duppage not implemented");
	pte_t pte = uvpt[pn];
	void * va = pn * PGSIZE;
	if ((pte & PTE_W) == PTE_W || (pte & PTE_COW) == PTE_COW) {
		if ((r = sys_page_map(0, va, envid, va,(PTE_U | PTE_P | PTE_COW))) < 0) {
			return r;
		}
		// UVPT cannot write by user, have to call kernel to map it
		if ((r = sys_page_map(0, va, 0, va, (PTE_U | PTE_P | PTE_COW))) < 0) {
			return r;
		} 
	} else {
		if ((r = sys_page_map(0, va, envid, va, pte & PTE_SYSCALL)) < 0) {
			return r;
		}
	}
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	// panic("fork not implemented");
	envid_t child_envid; 
	uint8_t *addr;
	int r; 
	int i;
	// Install page fault handler
	set_pgfault_handler(pgfault);

	// Create the child process;
	if ((child_envid = sys_exofork()) < 0) {
		panic("sys_exofork() failed.\n");
	} else if (child_envid == 0) {
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	// Parent's job: copy the address space
	// How to access the page directories and page tables in the user space
	// UVPT come to help
	for (addr = 0; addr < (uint8_t *)UTOP; addr += PGSIZE) {
		if (uvpd[PDX(addr)] & PTE_P) {
			if (uvpt[PGNUM(addr)] & PTE_P) {
				if (addr >= (uint8_t *)(UXSTACKTOP - PGSIZE) && addr < (uint8_t *)UXSTACKTOP) continue;
				r = duppage(child_envid, PGNUM(addr));
			}
		}
	}

	if ((r = sys_page_alloc(child_envid, (void *)(UXSTACKTOP - PGSIZE), PTE_U | PTE_P | PTE_W)) < 0) {
		return r;
	}

	// All done, register the pgfault with child process
	// cannot directly assign pgfault as the call, needs an additional wrapper upcall
	if ((r = sys_env_set_pgfault_upcall(child_envid, thisenv->env_pgfault_upcall)) < 0) {
		return r;
	}
	// Mark the child process as runnable
	if ((r = sys_env_set_status(child_envid, ENV_RUNNABLE)) < 0) {
		return r;
	}

	return child_envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
