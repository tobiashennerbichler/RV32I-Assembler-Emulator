# RV32I-Assembler-Emulator
This project implements a basic Assembler and Emulator for the RISC-V RV32I
instruction set. It is created for learning purposes and is thus not guaranteed to work correctly
for every used file. (Still in testing)

It currently works by starting the program with a file containing RISC-V RV32I instructions
which are then parsed through and assembled. The given program is then executed.

## Supported Instructions:

### U(pper Immediate) Types:
**AUIPC, LUI** \
Usage: INSTRUCTION rd, imm

### J(ump) Types:
**JAL** \
Usage: INSTRUCTION rd, label

### R Types:
**ADD, AND, OR, SLL, SLT, SLTU, SRA, SRL, SUB, XOR** \
Usage: INSTRUCTION rd, rs1, rs2

### I(mmediate) Types:
**ADDI, ANDI, ORI, SLTI, SLTIU, XORI** \
Usage: INSTRUCTION rd, rs1, imm

**JALR, LB, LBU, LH, LHU, LW** \
Usage: INSTRUCTION rd, imm(rs1)

**SLLI, SRAI, SRLI** \
Usage: INSTRUCTION rd, rs1, shamt \
(shamt = shift amount, only 5 bits)

### S(tore) Types:
**SB, SH, SW** \
Usage: INSTRUCTION rs2, imm(rs1)

### B(ranch) Types:
**BEQ, BGE, BGEU, BLT, BLTU, BNE** \
Usage: INSTRUCTION rs1, rs2, label

## Available Registers:

**x0  --> zero** \
**x1  --> ra** \
**x2  --> sp** \
**x3  --> gp** \
**x4  --> tp** \
**x5  --> t0** \
**x6  --> t1** \
**x7  --> t2** \
**x8  --> fp** \
**x9  --> s1** \
**x10 --> a0** \
**x11 --> a1** \
**x12 --> a2** \
**x13 --> a3** \
**x14 --> a4** \
**x15 --> a5** \
**x16 --> a6** \
**x17 --> a7** \
**x18 --> s2** \
**x19 --> s3** \
**x20 --> s4** \
**x21 --> s5** \
**x22 --> s6** \
**x23 --> s7** \
**x24 --> s8** \
**x25 --> s9** \
**x26 --> s10** \
**x27 --> s11** \
**x28 --> t3** \
**x29 --> t4** \
**x30 --> t5** \
**x31 --> t6**

