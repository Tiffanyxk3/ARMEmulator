/* find_max_index_s - find the maximum value in an interger array */

.global find_max_index_s

/* r0: int *array
 * r1: int len

 * r2: int i
 * r3: int max
 * r5: int index
 */
find_max_index_s:
	sub sp, sp, #12
	str lr, [sp]
	str r4, [sp, #4]          /* store r4 at #4 */
	str r5, [sp, #8]          /* store r5 at #8 */
	mov r5, #0
	ldr r3, [r0, r5, LSL #2]  /* r3 = array + (index * 4) */
	mov r2, #1
	
find_max_loop:
	cmp r2, r1
	bge find_max_endloop      /* checking for the loop condition */
	ldr r4, [r0, r2, LSL #2]  /* r4 = array + (i * 4) */
	cmp r4, r3                /* checking if the current value is the new maximum */
	bgt find_max_greater
	b find_max_loop_increment

find_max_greater:
/* updating values of max and index */
	mov r3, r4
	mov r5, r2

find_max_loop_increment:
/* repeated tracker incrementing block */
	add r2, r2, #1
	b find_max_loop

find_max_endloop:
	mov r0, r5
	ldr r4, [sp, #4]
	ldr r5, [sp, #8]
	ldr lr, [sp]
	add sp, sp, #12
	bx lr

