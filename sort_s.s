/* sort_s: sort int array in descending order */

.global sort_s

/* r0: int *array
 * r1: int len

 * r2: int i
 * r3: int max_index
 */
sort_s:
	sub sp, sp, #16
	str lr, [sp]
	mov r2, #0

sort_s_loop:
	cmp r2, r1
	bge sort_s_end

	str r0, [sp, #4]    /* store array at #4 */
	str r1, [sp, #8]    /* store len at #8 */
	str r2, [sp, #12]   /* store i at #12 */
	sub r1, r1, r2
	add r0, r0, r2, LSL #2
	bl find_max_index_s
	ldr r2, [sp, #12]
	add r3, r0, r2

	ldr r0, [sp, #4]
	cmp r3, r2
	bne sort_s_not_equal_i

	b sort_s_increment

sort_s_not_equal_i:
	ldr r12, [r0, r2, LSL #2]
	ldr r1, [r0, r3, LSL #2]
	str r1, [r0, r2, LSL #2]
	str r12, [r0, r3, LSL #2]

sort_s_increment:
	ldr r1, [sp, #8]
	add r2, r2, #1
	b sort_s_loop	

sort_s_end:
	mov r0, r1
	ldr lr, [sp]
	add sp, sp, #16
	bx lr
