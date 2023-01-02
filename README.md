# Arm-Thumb2-Malloc-and-Free

**Objective**
ARM assembly language level concept learning: 

• CPU operating modes: user and supervisor modes
• System-call handling procedures
• C to assembler argument passing (APCS: ARM Procedure Call Standard)
• Stack operations to implement recursions at the assembly language level
• Buddy memory allocation
• Hardware and Computer Organization at a lower level.

**Software:**

* Keil uVision -> https://www2.keil.com/mdk5/uvision/
* VisUAL -> https://salmanarif.bitbucket.io/visual/downloads.html
* Visual Studio Code

**Language:**
* Arm asssembly language
* Thumb2 assembly language
* C language

**Implementation files:**

startup_tm4c129.s: Investigate the Reset_Handler routine and implement SVC_Handler 
  
driver_keil.c: testing file (no change needed)

stdlib.: _bzero and _strncpy implementation

svc.s: system call jump from main to keil file for testing purpose.

heap.s : implement the following 5 routines, based on the C implementation
- _heap_init: mcb initialization
- _kalloc: the entry point to invoke the _ralloc recursive helper function
- _ralloc: a recursive helper function to allocate a space
- _kfree: the entry point to invoke the _rfree recursive helper function
- _rfree: a recursive helper function to free the space and merge the buddy space if possible


To run and test heap.c, you only need to compile and run heap.c as follow in a Linux system (C 
program):
gcc driver_cpg.c heap.c -o ./a.out
./a.out

More information:
[Project_Description.pdf](https://github.com/SnaraC/Arm-Thumb2-Malloc-and-Free/files/10330240/Project_Description.pdf)

*Made by Sovannara Chou*
