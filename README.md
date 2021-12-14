# ARM Emulator (C, Arm Assembly)

- This project is aimed to understand machine code and interpret the machine code instructions.
- An emulator for a subset of the ARMv7 Instruction Set Architecture (ISA) written in C.
- The emulator supports all instructions that appear in fib_rec_s.s, get_bitseq_s.s, is_pal_s.s , max3_s.s, merge_sort_s.s, quadratic_s.s, sort_s.s, to_upper_s.s, sort_s.s, find_max_index_s.s, merge_s.s, merge_sort_s.s.
- Dynamic analysis of instruction execution is supported. This includes:
  1. Number of instructions executed (i_count)
  2. Number of data processing  and mul instructions executed (dp_count)
  3. Number of SDT (memory) instructions executed (sdt_count)
  4. Number of branch instructions executed including b, bl, bx, bCC (b_count)
  5. Number of branches taken including b, bl, bx, bCC (b_taken)
  6. Number of branches not taken from bCC (b_not_taken)
- A 4-way set-associative cache is implemented

## Usage
*Prerequisite is setting up your enviornment to compile and run x86 code.*
1. `make`
2. `./project04 <prog> [<arg1> ...]`
For example, `./project04 sort 3 5 1 4`
