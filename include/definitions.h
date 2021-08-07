#pragma once

#include <cstdint>
#include <string>

#define OPCODE_MASK 0x7F
#define WIDTH 500
#define HEIGHT 300
#define PIXEL_SIZE 2
#define WHITE 0xFFFFFF
#define GREY 0x333333
#define BLUE 0xFF
#define BLACK 0

#define DEBUG

typedef uint32_t u32;
typedef int32_t s32;
typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;

enum InstructionType
{
  U_TYPE,
  J_TYPE,
  R_TYPE,
  I_TYPE,
  I_TYPE2,
  S_TYPE,
  B_TYPE
};

struct InstructionInfo
{
  u32 opcode_;
  u32 funct3_;
  u32 funct7_;
  InstructionType type_;
};

inline bool operator<(const InstructionInfo &i1, const InstructionInfo &i2)
{
  if(i1.opcode_ != i2.opcode_)
  {
    return i1.opcode_ < i2.opcode_;
  }
  else if(i1.funct3_ != i2.funct3_)
  {
    return i1.funct3_ < i2.funct3_;
  }
  else if(i1.funct7_ != i2.funct7_)
  {
    return i1.funct7_ < i2.funct7_;
  }
  else
  {
    return i1.type_ < i2.type_;
  }
}

struct Instruction
{
  std::string name_;
  u32 Rd_;
  u32 Rs1_;
  u32 Rs2_;
  u32 imm_;
  std::string label_name_;
  InstructionInfo info_;
};

struct Label
{
  bool verified_;
  u32 address_;
};

/*
 * opcodes:
 * EBREAK: 0x00
 * ADD: 0x01
 * ADDI: 0x02
 * SUB: 0x03
 * SW: 0x04
 * LW: 0x05
 * JAL: 0x06
 * JALR: 0x07
 * BEQ: 0x08
 * BNE: 0x09
 * BLT: 0x0A
 * BGE: 0x0B
 */