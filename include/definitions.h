#include <cstdint>
#include <string>

#define OPCODE_MASK 0x7F
#define RD_MASK 0xF80
#define RD_SHIFT 7
#define RS1_MASK 0xF8000
#define RS1_SHIFT 15
#define RS2_MASK 0x1F00000
#define RS2_SHIFT 20
#define IMM12_MASK 0xFFF00000
#define IMM12_SHIFT 20
#define IMM5_MASK 0xF80
#define IMM5_SHIFT 7
#define IMM7_MASK 0xFE000000
#define IMM7_SHIFT 25
#define IMM20_MASK 0xFFFFF000
#define IMM20_SHIFT 12
#define SIGN_MASK 0x80000000
#define SIGN_SHIFT 31

typedef uint32_t u32;
typedef int32_t s32;

//TODO: maybe change this
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