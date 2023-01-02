		AREA	|.text|, CODE, READONLY, ALIGN=2
		THUMB

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; System Call Table
HEAP_TOP	EQU		0x20001000
HEAP_BOT	EQU		0x20004FE0
MAX_SIZE	EQU		0x00004000		; 16KB = 2^14
MIN_SIZE	EQU		0x00000020		; 32B  = 2^5
	
MCB_TOP		EQU		0x20006800      ; 2^10B = 1K Space
MCB_BOT		EQU		0x20006BFE
MCB_ENT_SZ	EQU		0x00000002		; 2B per entry
MCB_TOTAL	EQU		512				; 2^9 = 512 entries
	
INVALID		EQU		-1				; an invalid id
	
;
; Each MCB Entry
; FEDCBA9876543210
; 00SSSSSSSSS0000U					S bits are used for Heap size, U=1 Used U=0 Not Used

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Memory Control Block Initialization
; void _heap_init( )
; this routine must be called from Reset_Handler in startup_TM4C129.s
; before you invoke main( ) in driver_keil
		EXPORT	_heap_init
_heap_init
		; you must correctly set the value of each MCB block
		; complete your code
		PUSH {r2-r12,lr} 
		LDR R2, =MCB_TOP 
		LDR R5, =MCB_BOT
		LDR R6, =HEAP_TOP
		LDR R8, =HEAP_BOT
		LDR R3, =MAX_SIZE
		STR R3, [R2], #4
		B loopMCB
		
loopMCB ; initialize MCB address 
		CMP R2, R5
		BEQ loopheap ;after done move to heap initialize loop
		MOV R4, #0
		STRB r4, [r2], #1
		B loopMCB
		
loopheap ; initialize heap address
		CMP R6, R8
		BEQ end ;after done finish the heap init function 
		MOV R4, #0
		STRB r4, [r6], #1
		B loopheap
		
end		
		POP{r2-r12,lr}	
		BX		lr ; branch to main

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Kernel Memory Allocation
; void* _k_alloc( int size )
		EXPORT	_kalloc
_kalloc
		; complete your code
		push {r1-r12, lr}
		LDR R1, =MCB_TOP
		LDR R2, =MCB_BOT
		B _ralloc
		
_ralloc ; recursive function for kalloc
		SUB R3, R2, R1  
		LDR R4, =MCB_ENT_SZ 
		ADD R3, R3, R4 ; entire mcb
		MOV R4, #2 
		SDIV R4, R3, R4 ; half val mcb
		ADD R5, R4, R1  ; address midpoint
		LDR R6, [R1] ; value of mcb top 
		TST R6, #1 ; check if it use or not
		BEQ _checkUnused ;if LSB is zero
		BNE _checkUsed	; if LSB is one
		
_rallocPop	pop {r1-r12,lr}
		BX		lr	
		
_checkUnused ; check if the space is available or unused
		TST R1, #1 
		CMP R0, R6
		BLT splitvalue ;split value to the right buddy 
		BEQ setvalue ;set value at address

_checkUsed ; check if the space is used or unavailable
		CMP R0, R6
		BLT checkLeft
		BGT checkRight
		
checkLeft ;check left buddy
		MOV R8, R6
		LDR R6, [R5]
		
		LDR R7, [R5]
		SUB R7, R7, #1
		CMP R7, R0
		BEQ Rightdivision
		
		CMP R0, R6
		MOVEQ R2, R5
		BEQ setvalueRight		
		BLT setMidtoBot
		BGT	setMidtoTop

Rightdivision
		MOV R4, #16
		SDIV R7, R7, R4
		ADD R5, R5, R7
		ADD R5, #1
		LDR R8, [R5]
		TST R8, #1
		MOVEQ R2, R5
		LDR R6, [R5]
		BEQ setvalueRight
		
		CMP R0, R6
		MOVEQ R2, R5
		BEQ setvalueRight		
		BLT setMidtoBot
		BGT	setMidtoTop
		
		
checkRight	;check right buddy
		MOV R8, R6
		LDR R6, [R5]
		CMP R0, R6
		MOVEQ R1, R5
		BEQ setvalue		
		BLT setMidtoBot
		BGT	setMidtoTop

setMidtoTop	;move the mid address to top/R1
		SUB R8, R8, #1
		LSL R8, #1
			
		CMP R8, R6
		BEQ checkMidRight			

		MOV R1, R5
		b _ralloc

setMidtoBot ; move the mid address to bottom/R2
		SUB R8, R8, #1
		LSL R8, #1
			
		CMP R8, R6
		BEQ checkMid
			
		MOV R2, R5
		b _ralloc


checkMid ;check midpoint or the right buddy of the top to see if it available
		LSR R8, #1
		ADD R8, R8, #1
		push {r3,r6}
		MOV r6, r5
		SUB r3, r5, r1
		LDR R4, =MCB_ENT_SZ
		ADD r3, r3, r4
		MOV R4, #2 
		SDIV R4, R3, R4 
		ADD R5, R4, R1
		LDR r3, [r5]
		CMP R3, R8
		MOVEQ R1, R6
		CMP R3, R8
		MOVNE R2, R6
		pop {r3,r6}
		b _ralloc

checkMidRight ;check midpoint or the left buddy of the mid to see if it available
		LSR R8, #1
		ADD R8, R8, #1
		push {r3,r6}
		MOV r6, r5
		SUB r3, r5, r1
		LDR R4, =MCB_ENT_SZ
		ADD r3, r3, r4
		MOV R4, #2 
		SDIV R4, R3, R4 
		ADD R5, R4, R1
		LDR r3, [r5]
		CMP R3, R8
		MOVEQ R2, R6
		CMP R3, R8
		MOVNE R1, R6
		pop {r3,r6}
		b _ralloc

		
splitvalue ;splitvalue into two left and right buddy
		MOV R8, #2
		SDIV R6, R6, R8
		STR R6, [R1]
		STR R6, [R5]
		MOV R2, R5
		B _ralloc
		
setvalue ; set value at fitted address
		ADD R6, R6, #1
		STR R6, [R1]
		STR R6, [R0] ; return value should be saved into r0
		B _rallocPop ; return to ralloc to main

setvalueRight ; set the value at the right buddy if left is not available
		ADD R6, R6, #1
		STR R6, [R2]
		STR R6, [R0] ; return value should be saved into r0
		B _rallocPop ; return to ralloc to main
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Kernel Memory De-allocation
; void *_kfree( void *ptr )	
		EXPORT	_kfree
_kfree
		; complete your code
		push {r1-r12, lr}
		LDR R1, =MCB_TOP
		LDR R2, =MCB_BOT		
		B _rfree
		
		; return value should be saved into r0	
	
_rfree ; rfree recursive function 
		LDR R3, [R1]
		CMP R3, #0x10000
		BGT lsrR3
		CMP R3, #0x100
		BLT lslR3
		TST R3, #1
		BEQ _Unused
		BNE _Used

lsrR3
		LSR R3, #8
		TST R3, #1
		BEQ _Unused
		BNE _Used
		
lslR3
		LSL R3, #8
		TST R3, #1
		BEQ _Unused
		BNE _Used
	
_Unused ;space unused we move on to the next address
		B moveOn

_Used ;space in use we check the value to see if it is the right one to free
		SUB R3, R3, #1
		CMP R0, R3
		BNE moveOn
		
		STR R3, [R1]
		LDR R4, =MCB_ENT_SZ
		MOV R9, R3
		ADD R3, R3, R4
		MOV R4, #16
		SDIV R6, R3, R4
		MOV R5, R1
		
		ADD R10, R5, R6
		ADD R10, R5, #1
		
		SUB R8, R5, R6
		SUB R8, R8, #1
		
		LDR R7, [R10]
		CMP R7, #0x10000
		BGT lslR7
		CMP R7, R9
		BEQ _MergeBuddy
		LDR R7, [R8]
		CMP R7, #0x10000
		BGT lslR7
		CMP R7, R9
		BEQ _MergeBuddyLeft
		BNE END

lslR7
		LSR R7, #8
		CMP R7, R9
		BEQ _MergeBuddy
		LDR R7, [R8]
		CMP R7, R9
		BEQ _MergeBuddyLeft
		BNE END

_MergeBuddyLeft ; merge left buddy
		LDR R6, [R5]
		LDR R7, [R1]
		MOV R9, #0
		ADD R6, R6, R7
		STR R6, [R8]
		STR R9, [R1]
		MOV R1, R8
		B RecursiveMerge
			
_MergeBuddy ;merge right buddy
		LDR R6, [R5]
		LDR R7, [R1]
		MOV R9, #0
		ADD R6, R6, R7
		STR R6, [R8]
		STR R9, [R1]
		LDR r3, =MCB_TOP
		CMP R8, R3
		BLT donotmove
		MOV R1, R8
		B RecursiveMerge
		
donotmove ; if above or below the mcb top and bot do not move anymore switch value for another recursive merge
		STR R6, [R1]
		STR R9, [R10]
		B RecursiveMerge
		
RecursiveMerge ;recursive merge to keep merging if the left and right space is the same size until everything is merge
		LDR r3, [r8]
		LDR R4, =MCB_ENT_SZ
		MOV R9, R3
		ADD R3, R3, R4
		MOV R4, #16
		SDIV R6, R3, R4
		
		ADD R10, R8, R6
		ADD R10, R10, #1
		
		SUB R8, R8, R6
		
		LDR R7, [R8]
		CMP R7, #0x10000
		BGT lslR7
		CMP R7, R9
		MOV R5, R8
		BEQ _MergeBuddyLeft	
		LDR R7, [R10]
		CMP R7, #0x10000
		BGT lslR7
		CMP R7, #100
		BLT checkevenindex
		CMP R7, R9
		MOV R5, R10
		BEQ _MergeBuddy

		B END

checkevenindex ; helper function to check the even index
		SUB R10, R10, #1
		LDR R7, [R10]
		CMP R7, R9
		MOV R5, R10
		BEQ _MergeBuddy
		LDR R7, [R8]
		CMP R7, #0x10000
		BGT lslR7
		CMP R7, R9
		MOV R5, R8
		BEQ _MergeBuddyLeft	
		B END

moveOn ; move to the next address
		MOV R5, R3
		LDR R4, =MCB_ENT_SZ
		ADD R5, R5, R4
		MOV R4, #16
		SDIV R6, R5, R4
		ADD R1, R1, R6
		TST R1, #1
		ADDEQ R1, R1, #1
		LDR R3, [R1]
		CMP R3, #100
		BLT moveoncheckevenorodd
		B _rfree
		
moveoncheckevenorodd 
		SUB R1, R1, #1
		B _rfree

END	
		pop {r1-r12,lr}
		BX		lr		
