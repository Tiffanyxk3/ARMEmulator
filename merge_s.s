@ merge_s.s - Given the array length followed by an array containing two sub-arrays in sorted order, merge combines the sub-arrays.

.global merge_s

@ r0 - a[]
@ r1 - aux[]
@ r2 - start
@ r3 - end

@ r4 - mid
@ r5 - pointer_left
@ r6 - pointer_right
@ r7 - i
@ r8 - temp1
@ r9 - temp2

merge_s:
    sub sp, sp, #32
    str lr, [sp]
    str r4, [sp, #4]
    str r5, [sp, #8]
    str r6, [sp, #12]
    str r7, [sp, #16]
    str r8, [sp, #20]
    str r9, [sp, #24]

    add r4, r2, r3
    lsr r4, r4, #1
    mov r5, r2
    add r6, r4, #1
    mov r7, r2 @ i = start
    b merge_s_loop1

merge_s_loop1:
    cmp r7, r3 @ check for the loop condition
    bgt merge_s_endloop1

    @ check for the first case
    add r8, r4, #1
    cmp r5, r8
    beq merge_s_left_limit

    @ check for the second case
    add r8, r3, #1
    cmp r6, r8
    beq merge_s_right_limit

    @ check for the third case
    ldr r8, [r0, r5, lsl #2]
    ldr r9, [r0, r6, lsl #2]
    cmp r8, r9
    blt merge_s_left_smaller

    @ last case: pointer right points to smaller element
    ldr r8, [r0, r6, lsl #2]
    str r8, [r1, r7, lsl #2]
    add r6, r6, #1
    b merge_s_increment

merge_s_left_limit: @ left pointer has reached the limit
    ldr r8, [r0, r6, lsl #2]
    str r8, [r1, r7, lsl #2]
    add r6, r6, #1
    b merge_s_increment

merge_s_right_limit: @ right pointer has reached the limit
    ldr r8, [r0, r5, lsl #2]
    str r8, [r1, r7, lsl #2]
    add r5, r5, #1
    b merge_s_increment

merge_s_left_smaller: @ pointer left points to smaller element
    ldr r8, [r0, r5, lsl #2]
    str r8, [r1, r7, lsl #2]
    add r5, r5, #1
    b merge_s_increment

merge_s_increment:
    add r7, r7, #1
    b merge_s_loop1

merge_s_endloop1:
    mov r7, r2 @ reset i = start
    b merge_s_loop2

merge_s_loop2: @ copy the elements from aux[] to a[]
    cmp r7, r3
    bgt merge_s_end
    ldr r8, [r1, r7, lsl #2]
    str r8, [r0, r7, lsl #2]
    add r7, r7, #1
    b merge_s_loop2

merge_s_end:
    ldr r4, [sp, #4]
    ldr r5, [sp, #8]
    ldr r6, [sp, #12]
    ldr r7, [sp, #26]
    ldr r8, [sp, #20]
    ldr r9, [sp, #24]
    ldr lr, [sp]
    add sp, sp, #32
    bx lr

