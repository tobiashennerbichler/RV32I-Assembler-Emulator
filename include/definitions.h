#include <cstdint>
#include <string>

#define OPCODE_MASK 0x7F

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
  B_TYPE,
  NONE
};

struct Instruction
{
  std::string name_;
  int Rd_;
  int Rs1_;
  int Rs2_;
  int imm_;
  std::string label_name_;
};

struct Label
{
  bool verified_;
  u32 address_;
};

struct InstructionInfo
{
  int opcode_;
  int func3_;
  int func7_;
  InstructionType type_;
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