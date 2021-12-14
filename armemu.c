#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "armemu.h"

// Analysis functions

// Project04: Analysis struct init
void analysis_init(struct analysis_st *ap) {
    ap->i_count = 0;
    ap->dp_count = 0;
    ap->mem_count = 0;
    ap->b_count = 0;
    ap->b_taken = 0;
    ap->b_not_taken = 0;
}

// Project04: Print results of dynamic analysis
void analysis_print(struct analysis_st *ap) {
    printf("=== Analysis\n");
    printf("I_count       = %d\n", ap->i_count);
    printf("DP_count      = %d (%.2f%%)\n", ap->dp_count,
           ((double) ap->dp_count / (double) ap->i_count) * 100.0);
    printf("SDT_count     = %d (%.2f%%)\n", ap->mem_count,
           ((double) ap->mem_count / (double) ap->i_count) * 100.0);    
    printf("B_count       = %d (%.2f%%)\n", ap->b_count,
           ((double) ap->b_count / (double) ap->i_count) * 100.0);
    printf("B_taken       = %d (%.2f%%)\n", ap->b_taken,
               ((double) ap->b_taken / (double) ap->b_count) * 100.0);
    printf("B_not_taken   = %d (%.2f%%)\n", ap->b_not_taken,
               ((double) ap->b_not_taken / (double) ap->b_count) * 100.0);
}

// Project04: Print results of dynamic analysis and cache sim
void armemu_print(struct arm_state *asp) {
    if (asp->analyze) {
        analysis_print(&(asp->analysis));
    }
    if (asp->cache_sim) {
        cache_print((&asp->cache));
    }
}

void armemu_init(struct arm_state *asp, uint32_t *func, uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3) {
    int i;

    // Zero out registers
    for (i = 0; i < NREGS; i += 1) {
        asp->regs[i] = 0;
    }

    // Zero out the CPSR
    asp->cpsr = 0;

    // Zero out the stack
    for (i = 0; i < STACK_SIZE; i += 1) {
        asp->stack[i] = 0;
    }

    // Initialize the Program Counter
    asp->regs[PC] = (uint32_t) func;

    // Initialize the Link Register to a sentinel value
    asp->regs[LR] = 0;

    // Initialize Stack Pointer to the logical bottom of the stack
    asp->regs[SP] = (uint32_t) &asp->stack[STACK_SIZE];

    // Initialize the first 4 arguments in emulated r0-r3
    asp->regs[0] = a0;
    asp->regs[1] = a1;
    asp->regs[2] = a2;
    asp->regs[3] = a3;

    // Project04: Initialize dynamic analysis
    analysis_init(&asp->analysis);
    
    // Project04: Initialize cache simulator
    cache_init(&asp->cache);
}


bool armemu_is_bx(uint32_t iw) {
    uint32_t bxcode = (iw >> 4) & 0xFFFFFF;

    return bxcode == 0x12fff1;   
}

void armemu_bx(struct arm_state *asp, uint32_t iw) {
    uint32_t rn = iw & 0xF;

    // Project04: increment dynamic analysis
    asp->analysis.b_count += 1;
    asp->analysis.b_taken += 1;

    asp->regs[PC] = asp->regs[rn];    
}

bool armemu_is_branch(uint32_t iw) {
    uint32_t branch_code = (iw >> 25) & 0b111;

    return branch_code == 0b101;
}

void armemu_b(struct arm_state * asp, uint32_t iw) {
    uint32_t condition, offset;
    uint32_t cpsr = asp->cpsr;

    condition = (iw >> 28) & 0xF;

    offset = iw & 0xFFFFFF;
    offset = offset << 2;
    if ((offset >> 23) & 0b1) {
        for (int i = 26; i < 32; i++) {
            offset = (0b1 << i) | offset;
        }
    }

    bool branch = false;
    if (condition == 0b1110) { // always
        branch = true;
    } else if (condition == 0b0000 && (int) cpsr == 0) { // EQ
        branch = true;
    } else if (condition == 0b0001 && (int) cpsr != 0) { // NE
        branch = true;
    } else if (condition == 0b1100 && (int) cpsr > 0) {  // GT
        branch = true;
    } else if (condition == 0b1010 && (int) cpsr >= 0) { // GE
        branch = true;
    } else if (condition == 0b1011 && (int) cpsr < 0) {  // LT
        branch = true;
    } else if (condition == 0b1101 && (int) cpsr <= 0) { // LE
        branch = true;
    }

    if (branch) {
        asp->regs[PC] = asp->regs[PC] + 4 + offset;
        asp->analysis.b_taken += 1;
    } else{
        asp->analysis.b_not_taken += 1;
    }

    asp->regs[PC] = asp->regs[PC] + 4;
}

void armemu_bl(struct arm_state * asp, uint32_t iw) {
    uint32_t offset = iw & 0xFFFFFF;

    asp->analysis.b_taken += 1;

    offset = offset << 2;
    if ((offset >> 23) & 0b1) {
        for (int i = 26; i < 32; i++) {
            offset = (0b1 << i) | offset;
        }
    }
    offset += 8;

    asp->regs[LR] = asp->regs[PC] + 4;
    asp->regs[PC] = asp->regs[PC] + offset;
}

void armemu_branch(struct arm_state *asp, uint32_t iw) {
    uint32_t l_bit = (iw >> 24) & 0b1;

    asp->analysis.b_count += 1;

    if (l_bit) {
        armemu_bl(asp, iw);
    } else {
        armemu_b(asp, iw);
    }
}

bool armemu_is_add(uint32_t iw) {
    uint32_t dp_bits = (iw >> 26) & 0b11;
    uint32_t opcode = (iw >> 21) & 0xF;

    return (dp_bits == 0b00) & (opcode == 0b0100);
}

void armemu_add(struct arm_state *asp, uint32_t iw) {
    uint32_t i_bit = (iw >> 25) & 0b1;
    uint32_t rn = (iw >> 16) & 0xF;
    uint32_t rd = (iw >> 12) & 0xF;
    uint32_t oper2;

    // Project04: Increment analysis count
    asp->analysis.dp_count += 1;
    
    if (!i_bit) { // register
        uint32_t rm = iw & 0xF;
        uint32_t shift = (iw >> 4) & 0xFF;
        uint32_t amount = (iw >> 7) & 0b11111;
        uint32_t type = (iw >> 5) & 0b11;

        if (shift != 0) {
            if (type == 0b00) { // logical left
                oper2 = asp->regs[rm] << amount;
                // oper2 = asp->regs[rm] + shift;
            } else if (type == 0b01) { // logical right
                oper2 = asp->regs[rm] >> amount;
            } else if (type == 0b10) { // arithmetic right
                oper2 = (int) asp->regs[rm] >> (int) amount;
            }
        } else {
            oper2 = asp->regs[rm];
        }
    } else { // immediate
        uint32_t imm = iw & 0xFF;
        oper2 = imm;
    }

    asp->regs[rd] = asp->regs[rn] + oper2;

    if (rd != PC) {
        asp->regs[PC] = asp->regs[PC] + 4;
    }
}

bool armemu_is_mov(uint32_t iw) {
    uint32_t dp_bits = (iw >> 26) & 0b11;
    uint32_t opcode = (iw >> 21) & 0xF;

    return (dp_bits == 0b00) & (opcode == 0b1101);
}

void armemu_mov(struct arm_state * asp, uint32_t iw) {
    uint32_t i_bit = (iw >> 25) & 0b1;
    uint32_t rd = (iw >> 12) & 0xF;
    uint32_t rm = iw & 0xF;
    uint32_t imm = iw & 0xFF;
    uint32_t oper2;

    asp->analysis.dp_count += 1;

    if (!i_bit) {
        uint32_t shift = (iw >> 4) & 0xFF;
        if (shift != 0) {
            uint32_t field = (iw >> 4) & 0b1;
            uint32_t type = (iw >> 5) & 0b11;
            uint32_t amount = (iw >> 7) & 0b11111;
            if (field == 0) {
                if (type == 0b00) { // logical left
                    oper2 = asp->regs[rm] << amount;
                } else if (type == 0b01) { // logical right
                    oper2 = asp->regs[rm] >> amount;
                } else if (type == 0b10) { // arithmetic right
                    oper2 = (int) asp->regs[rm] >> (int) amount;
                } else { // rotate right

                }
            } else { // field == 1
                uint32_t rs = (iw >> 8) & 0xF;
                if (type == 0b00) { // logical left
                    oper2 = asp->regs[rm] << asp->regs[rs];
                } else if (type == 0b01) { // logical right
                    oper2 = asp->regs[rm] >> asp->regs[rs];
                } else if (type == 0b10) { // arithmetic right
                    oper2 = (int) asp->regs[rm] >> (int) asp->regs[rs];
                } else { // rotate right,
                    
                }
            }
        } else {
            oper2 = asp->regs[rm];
        }
    } else {
        oper2 = imm;
    }

    asp->regs[rd] = oper2;

    if (rd != PC) {
        asp->regs[PC] = asp->regs[PC] + 4;
    }
}

bool armemu_is_sub(uint32_t iw) {
    uint32_t dp_bits = (iw >> 26) & 0b11;
    uint32_t opcode = (iw >> 21) & 0xF;

    return (dp_bits == 0b00) & (opcode == 0b0010);
}

void armemu_sub(struct arm_state * asp, uint32_t iw) {
    uint32_t i_bit = (iw >> 25) & 0b1;
    uint32_t rn = (iw >> 16) & 0xF;
    uint32_t rd = (iw >> 12) & 0xF;
    uint32_t rm = iw & 0xF;
    uint32_t imm = iw & 0xFF;
    uint32_t oper2;

    asp->analysis.dp_count += 1;

    if (!i_bit) {
        oper2 = asp->regs[rm];
    } else {
        oper2 = imm;
    }

    asp->regs[rd] = asp->regs[rn] - oper2;

    if (rd != PC) {
        asp->regs[PC] = asp->regs[PC] + 4;
    }
}

bool armemu_is_mul(uint32_t iw) {
    uint32_t bits1 = (iw >> 22) & 0b111111;
    uint32_t bits2 = (iw >> 4) & 0xF;

    return (bits1 == 0b000000) & (bits2 == 0b1001);
}

void armemu_mul(struct arm_state * asp, uint32_t iw) {
    uint32_t rd = (iw >> 16) & 0xF;
    uint32_t rs = (iw >> 8) & 0xF;
    uint32_t rm = iw & 0xF;

    asp->analysis.dp_count += 1;

    asp->regs[rd] = asp->regs[rm] * asp->regs[rs];

    if (rd != PC) {
        asp->regs[PC] = asp->regs[PC] + 4;
    }
}

bool armemu_is_cmp(uint32_t iw) {
    uint32_t dp_bits = (iw >> 26) & 0b11;
    uint32_t opcode = (iw >> 21) & 0xF;

    return (dp_bits == 0b00) & (opcode == 0b1010);
}

void armemu_cmp(struct arm_state * asp, uint32_t iw) {
    uint32_t i_bit = (iw >> 25) & 0b1;
    uint32_t rn = (iw >> 16) & 0xF;
    uint32_t rd = (iw >> 12) & 0xF;
    uint32_t rm = iw & 0xF;
    uint32_t imm = iw & 0xFF;
    uint32_t oper2;

    asp->analysis.dp_count += 1;

    if (!i_bit) {
        oper2 = asp->regs[rm];
    } else {
        oper2 = imm;
    }

    asp->cpsr = asp->regs[rn] - oper2;

    if (rd != PC) {
        asp->regs[PC] = asp->regs[PC] + 4;
    }
}

bool armemu_is_rsb(uint32_t iw) {
    uint32_t dp_bits = (iw >> 26) & 0b11;
    uint32_t opcode = (iw >> 21) & 0xF;

    return (dp_bits == 0b00) & (opcode == 0b0011);
}

void armemu_rsb(struct arm_state * asp, uint32_t iw) {
    uint32_t i_bit = (iw >> 25) & 0b1;
    uint32_t rn = (iw >> 16) & 0xF;
    uint32_t rd = (iw >> 12) & 0xF;
    uint32_t rm = iw & 0xF;
    uint32_t imm = iw & 0xFF;
    uint32_t oper2;

    asp->analysis.dp_count += 1;

    if (!i_bit) {
        oper2 = asp->regs[rm];
    } else {
        oper2 = imm;
    }

    asp->regs[rd] = oper2 - asp->regs[rn];

    if (rd != PC) {
        asp->regs[PC] = asp->regs[PC] + 4;
    }
}

bool armemu_is_and(uint32_t iw) {
    uint32_t dp_bits = (iw >> 26) & 0b11;
    uint32_t opcode = (iw >> 21) & 0xF;

    return (dp_bits == 0b00) & (opcode == 0b0000);
}

void armemu_and(struct arm_state *asp, uint32_t iw) {
    uint32_t i_bit = (iw >> 25) & 0b1;
    uint32_t rn = (iw >> 16) & 0xF;
    uint32_t rd = (iw >> 12) & 0xF;
    uint32_t oper2;

    asp->analysis.dp_count += 1;
    
    if (!i_bit) {
        uint32_t rm = iw & 0xF;
        oper2 = asp->regs[rm];
    } else {
        uint32_t imm = iw & 0xFF;
        oper2 = imm;
    }

    asp->regs[rd] = asp->regs[rn] & oper2;

    if (rd != PC) {
        asp->regs[PC] = asp->regs[PC] + 4;
    }
}

bool armemu_is_sdt(uint32_t iw) {
    uint32_t sdt_bits = (iw >> 26) & 0b11;

    return sdt_bits == 0b01;
}

void armemu_sdt(struct arm_state *asp, uint32_t iw) {
    uint32_t i_bit = (iw >> 25) & 0b1;
    uint32_t b_bit = (iw >> 22) & 0b1;
    uint32_t l_bit = (iw >> 20) & 0b1;
    uint32_t rd = (iw >> 12) & 0xF;
    uint32_t rn = (iw >> 16) & 0xF;
    uint32_t target_addr;

    asp->analysis.mem_count += 1;

    if (!i_bit) { // immediate
        uint32_t offset = iw & 0xFFF;
        target_addr = asp->regs[rn] + offset;
        if (l_bit) { // ldr
            asp->regs[rd] = *((uint32_t *) target_addr);
        } else { // str
            *((uint32_t *) target_addr) = asp->regs[rd];
        }
    } else { // register
        uint32_t rm = iw & 0xF;
        uint32_t shift = (iw >> 4) & 0xFF;

        uint32_t amount = (iw >> 7) & 0b11111;
        uint32_t type = (iw >> 5) & 0b11;

        if (shift != 0) {
            if (type == 0b00) { // logical left
                target_addr = asp->regs[rm] << amount;
            } else if (type == 0b01) { // logical right
                target_addr = asp->regs[rm] >> amount;
            } else if (type == 0b10) { // arithmetic right
                target_addr = (int) asp->regs[rm] >> (int) amount;
            }
        } else {
            target_addr = asp->regs[rm];
        }

        if (l_bit) { // ldr
            if (!b_bit) { // b_bit = 0: word quantity
                asp->regs[rd] = *((uint32_t *) (asp->regs[rn] + target_addr));
            } else { // b_bit = 1: byte quantity
                asp->regs[rd] = *((uint8_t *) (asp->regs[rn] + target_addr));
            }
        } else { // str
            if (!b_bit) { // b_bit = 0: word quantity
                *((uint32_t *) (asp->regs[rn] + target_addr)) = asp->regs[rd];
            } else { // b_bit = 1: byte quantity
                *((uint8_t *) (asp->regs[rn] + target_addr)) = asp->regs[rd];
            }
        }
    }

    if (rd != PC) {
        asp->regs[PC] = asp->regs[PC] + 4;
    }
}

void armemu_one(struct arm_state *asp) {

    asp->analysis.i_count += 1;

    /* Project04: get instruction word from instruction cache
       instead of the PC pointer directly
       uint32_t iw = *((uint32_t *) asp->regs[PC]);
    */
    uint32_t iw = cache_lookup(&asp->cache, asp->regs[PC]);

    // Order matters: more constrained to least constrained
    if (armemu_is_bx(iw)) {
        armemu_bx(asp, iw);
    } else if (armemu_is_branch(iw)) {
        armemu_branch(asp, iw);
    } else if (armemu_is_add(iw)) {
        armemu_add(asp, iw);
    } else if (armemu_is_mov(iw)) {
        armemu_mov(asp, iw);
    } else if (armemu_is_sub(iw)) {
        armemu_sub(asp, iw);
    } else if (armemu_is_mul(iw)) {
        armemu_mul(asp, iw);
    } else if (armemu_is_cmp(iw)) {
        armemu_cmp(asp, iw);
    } else if (armemu_is_rsb(iw)) {
        armemu_rsb(asp, iw);
    } else if (armemu_is_and(iw)) {
        armemu_and(asp, iw);
    } else if (armemu_is_sdt(iw)) {
        armemu_sdt(asp, iw);
    } else {
        printf("armemu_one() invalid instruction\n");
        exit(-1);
    }
}

int armemu(struct arm_state *asp) {
    while (asp->regs[PC] != 0) {
        armemu_one(asp);
    }
    
    return (int) asp->regs[0];
}

