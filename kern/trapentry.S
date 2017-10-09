/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

#define H_NAME(N)	hdl##N
#define T(N) 		TRAPHANDLER(H_NAME(N), N)
#define TN(N)		TRAPHANDLER_NOEC(H_NAME(N), N)

// 0, 1, 3 - 7 CPU does not push an error code (NO)
TN(0);
TN(1);
TN(2);
TN(3);
TN(4);
TN(5);
TN(6);
TN(7);

// 8 CPU always pushes 0
T(8);

// 9 NO
//TN(9);

// 10 - 14 YES
T(10);
T(11);
T(12);
T(13);
T(14);

// T(15);

// 16 NO
TN(16);

T(17);
TN(18);
TN(19);

TN(48);

/*
 * Lab 3: Your code here for _alltraps
 */
.globl _alltraps
_alltraps:
	pushl 	%ds; 	/* push ds */
	pushl 	%es;	/* push es */
	pushal; 		/* push all general registers in PushRegs */

	movl	$(GD_KD), %eax;
	movw 	%ax, %ds;
	movw 	%ax, %es;
	pushl 	%esp;
	call 	trap;
	/* the call trap never return. */
spin:
	jmp		spin;