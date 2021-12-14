@ merge_sort_s.s - Given the array length followed by array values, merge_sort recursively sorts the array in ascending order.

.global merge_sort_s

@ r0 - a[]
@ r1 - aux[]
@ r2 - start
@ r3 - end

@ r12 - mid

merge_sort_s:
    sub sp, sp, #24
    str lr, [sp]

    cmp r3, r2
    ble merge_sort_s_end @ the subsection is empty or a single element

    add r12, r2, r3
    lsr r12, r12, #1

    @ sort the left sub-array recursively
    str r0, [sp, #4]
    str r1, [sp, #8]
    str r2, [sp, #12]
    str r3, [sp, #16]
    str r12, [sp, #20]
    mov r3, r12
    bl merge_sort_s

    @ sort the right sub-array recursively
    ldr r0, [sp, #4]
    ldr r1, [sp, #8]
    ldr r2, [sp, #12]
    ldr r3, [sp, #16]
    ldr r12, [sp, #20]
    add r12, r12, #1
    mov r2, r12
    bl merge_sort_s

    @ merge the left and right sides
    ldr r0, [sp, #4]
    ldr r1, [sp, #8]
    ldr r2, [sp, #12]
    ldr r3, [sp, #16]
    bl merge_s

merge_sort_s_end:
    ldr lr, [sp]
    add sp, sp, #24
    bx lr

