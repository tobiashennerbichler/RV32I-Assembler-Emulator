#pragma once

#include <cstdint>
#include <string>

#define OPCODE_MASK 0x7F

#define NUMBER_WIDTH 5
#define NUMBER_HEIGHT 7
#define START_X 1
#define START_Y 1

#define BYTE_PER_LINE 16
#define X_LEN 32
#define HI_LO_DISTANCE (NUMBER_WIDTH + 1)
#define BYTE_DISTANCE (2 * NUMBER_WIDTH)
#define ADDRESS_DISTANCE (4 * NUMBER_WIDTH)
#define ROW_DISTANCE (NUMBER_HEIGHT + 2)
#define PC_SECTION 0x60
#define SP_SECTION 0x80

//start + space for address + space for 16 bytes + distance to register start
#define REGISTER_X (START_X + (X_LEN / 8) * (HI_LO_DISTANCE + BYTE_DISTANCE) + ADDRESS_DISTANCE + \
  BYTE_PER_LINE * (HI_LO_DISTANCE + BYTE_DISTANCE) + BYTE_DISTANCE + ADDRESS_DISTANCE - NUMBER_WIDTH)
#define MAX_X (REGISTER_X + 4 * HI_LO_DISTANCE + BYTE_DISTANCE + \
  (X_LEN / 8) * (HI_LO_DISTANCE + BYTE_DISTANCE) - NUMBER_WIDTH)

#define SP_Y (START_Y + ROW_DISTANCE + (((PC_SECTION * 2) / BYTE_PER_LINE) + 2) * ROW_DISTANCE)
#define SP_END_Y (SP_Y + ROW_DISTANCE + (((SP_SECTION * 2) / BYTE_PER_LINE) + 1) * ROW_DISTANCE)
#define REGISTER_END_Y (START_Y + ROW_DISTANCE + X_LEN * ROW_DISTANCE)
#define MAX_Y ((SP_END_Y < REGISTER_END_Y) ? REGISTER_END_Y : SP_END_Y)

#define WIDTH MAX_X
#define HEIGHT MAX_Y
#define PIXEL_SIZE 2

#define BMP_HEADER_SIZE 54

#define WHITE 0xFFFFFF
#define GREEN 0x00FF00
#define YELLOW 0xFFFF00
#define GREY 0x333333

typedef uint32_t u32;
typedef int32_t s32;
typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef std::vector<int> Pattern;

enum Highlight
{
  NONE = WHITE,
  PC = GREEN,
  CHANGED = YELLOW
};

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