#include <cassert>
#include "../include/CPU.h"

CPU::CPU() : memory_(0x10000), pc_(0)
{
  //used to get correct function for read instruction
  instruction_lut_ = {
      {{0x73, 0x00, 0x00, I_TYPE}, &CPU::EBREAK},
      {{0x37, 0x00, 0x00, U_TYPE}, &CPU::LUI},
      {{0x27, 0x00, 0x00, U_TYPE}, &CPU::AUIPC},
      {{0x6F, 0x00, 0x00, J_TYPE}, &CPU::JAL},
      {{0x67, 0x00, 0x00, I_TYPE}, &CPU::JALR},
      {{0x63, 0x00, 0x00, B_TYPE}, &CPU::BEQ},
      {{0x63, 0x01, 0x00, B_TYPE}, &CPU::BNE},
      {{0x63, 0x04, 0x00, B_TYPE}, &CPU::BLT},
      {{0x63, 0x05, 0x00, B_TYPE}, &CPU::BGE},
      {{0x63, 0x06, 0x00, B_TYPE}, &CPU::BLTU},
      {{0x63, 0x07, 0x00, B_TYPE}, &CPU::BGEU},
      {{0x03, 0x00, 0x00, I_TYPE}, &CPU::LB},
      {{0x03, 0x01, 0x00, I_TYPE}, &CPU::LH},
      {{0x03, 0x02, 0x00, I_TYPE}, &CPU::LW},
      {{0x03, 0x04, 0x00, I_TYPE}, &CPU::LBU},
      {{0x03, 0x05, 0x00, I_TYPE}, &CPU::LHU},
      {{0x23, 0x00, 0x00, S_TYPE}, &CPU::SB},
      {{0x23, 0x01, 0x00, S_TYPE}, &CPU::SH},
      {{0x23, 0x02, 0x00, S_TYPE}, &CPU::SW},
      {{0x13, 0x00, 0x00, I_TYPE}, &CPU::ADDI},
      {{0x13, 0x02, 0x00, I_TYPE}, &CPU::SLTI},
      {{0x13, 0x03, 0x00, I_TYPE}, &CPU::SLTIU},
      {{0x13, 0x04, 0x00, I_TYPE}, &CPU::XORI},
      {{0x13, 0x06, 0x00, I_TYPE}, &CPU::ORI},
      {{0x13, 0x07, 0x00, I_TYPE}, &CPU::ANDI},
      {{0x13, 0x01, 0x00, I_TYPE2}, &CPU::SLLI},
      {{0x13, 0x05, 0x00, I_TYPE2}, &CPU::SRLI},
      {{0x13, 0x05, 0x20, I_TYPE2}, &CPU::SRAI},
      {{0x33, 0x00, 0x00, R_TYPE}, &CPU::ADD},
      {{0x33, 0x00, 0x20, R_TYPE}, &CPU::SUB},
      {{0x33, 0x01, 0x00, R_TYPE}, &CPU::SLL},
      {{0x33, 0x02, 0x00, R_TYPE}, &CPU::SLT},
      {{0x33, 0x03, 0x00, R_TYPE}, &CPU::SLTU},
      {{0x33, 0x04, 0x00, R_TYPE}, &CPU::XOR},
      {{0x33, 0x05, 0x00, R_TYPE}, &CPU::SRL},
      {{0x33, 0x05, 0x20, R_TYPE}, &CPU::SRA},
      {{0x33, 0x06, 0x00, R_TYPE}, &CPU::OR},
      {{0x33, 0x07, 0x00, R_TYPE}, &CPU::AND}
  };

  for(auto &reg : registers_)
  {
    reg = 0;
  }
}

CPU::~CPU()
{

}

/*
 * Read one byte from the given address in memory
 */
u8 CPU::read(u32 address)
{
  return memory_.at(address);
}

/*
 * Write one byte to the given address in memory
 */
void CPU::write(u32 address, u8 data)
{
  memory_.at(address) = data;
}

/*
 * Fetches and executes one instruction from memory
 * Machine words stored in little endian
 */
void CPU::tick()
{
  u32 word = 0;
  u32 addr = pc_;

  for(int i = 0; i < 4; i++)
  {
    word |= (read(addr++) << i*8);
  }

  printf("read word: 0x%x from: 0x%x\n", word, pc_);

  for(int i = 0; i < 32; i++)
  {
    printf("reg[%d] = %d ", i, registers_[i]);

    if(((i + 1) % 8) == 0)
    {
      printf("\n");
    }
  }

  InstructionInfo info = getInfo(word);
  if((this->*instruction_lut_.at(info))(word))
  {
    pc_ += 4;
  }
}

/*
 * Uses the fetched machine word to reconstruct the Instruction Info
 * used to look up the correct instruction function
 */
InstructionInfo CPU::getInfo(u32 word)
{
  u32 opcode = (word & 0x7F);
  InstructionInfo info;

  //U Types
  if(opcode == 0x37 || opcode == 0x17)
  {
    info = {opcode, 0x00, 0x00, U_TYPE};
  }
  //J Types
  else if(opcode == 0x6F)
  {
    info = {opcode, 0x00, 0x00, J_TYPE};
  }
  //R Types
  else if(opcode == 0x33)
  {
    u32 func3 = ((word >> 12) & 0x7);
    u32 func7 = ((word >> 25) & 0x7F);
    info = {opcode, func3, func7, R_TYPE};
  }
  //I Types
  else if(opcode == 0x67 || opcode == 0x03)
  {
    u32 func3 = ((word >> 12) & 0x7);
    info = {opcode, func3, 0x00, I_TYPE};
  }
  //I Types shared with I Types 2 that use shamt
  else if(opcode == 0x13)
  {
    u32 func3 = ((word >> 12) & 0x7);

    if(func3 == 0x01 || func3 == 0x05)
    {
      u32 func7 = ((word >> 25) & 0x7F);
      info = {opcode, func3, func7, I_TYPE2};
    }
    else
    {
      info = {opcode, func3, 0x00, I_TYPE};
    }
  }
  //S Types
  else if(opcode == 0x23)
  {
    u32 func3 = ((word >> 12) & 0x7);
    info = {opcode, func3, 0x00, S_TYPE};
  }
  //B Types
  else if(opcode == 0x63)
  {
    u32 func3 = ((word >> 12) & 0x7);
    info = {opcode, func3, 0x00, B_TYPE};
  }
  //EBREAK
  else if(opcode == 0x73)
  {
    info = {opcode, 0x00, 0x00, I_TYPE};
  }
  else
  {
    printf("unknown instruction: 0x%x\n", word);
    exit(-1);
  }

  return info;
}

/*
 * The following functions are used to parse the necessary arguments from the machine word
 */
void CPU::getUTypeArgs(u32 word, u32 &rd_number, u32 &imm)
{
  rd_number = (word >> 7) & 0x1F;
  imm = word & 0xFFFFF000;
}

void CPU::getJTypeArgs(u32 word, u32 &rd_number, u32 &imm)
{
  rd_number = (word >> 7) & 0x1F;
  imm = (word & 0xFF000) | ((word >> 9) & 0x800) | ((word >> 20) & 0x7FE);

  imm = extendBit(imm, 20);
}

void CPU::getRTypeArgs(u32 word, u32 &rd_number, u32 &rs1_number, u32 &rs2_number)
{
  rd_number = ((word >> 7) & 0x1F);
  rs1_number = ((word >> 15) & 0x1F);
  rs2_number = ((word >> 20) & 0x1F);
}

void CPU::getITypeArgs(u32 word, u32 &rd_number, u32 &rs1_number, u32 &imm)
{
  rd_number = ((word >> 7) & 0x1F);
  rs1_number = ((word >> 15) & 0x1F);
  imm = ((word >> 20) & 0xFFF);

  imm = extendBit(imm, 11);
}

void CPU::getI2TypeArgs(u32 word, u32 &rd_number, u32 &rs1_number, u32 &shamt)
{
  rd_number = ((word >> 7) & 0x1F);
  rs1_number = ((word >> 15) & 0x1F);
  shamt = ((word >> 20) & 0x1F);
}

void CPU::getSTypeArgs(u32 word, u32 &rs1_number, u32 &rs2_number, u32 &imm)
{
  rs1_number = ((word >> 15) & 0x1F);
  rs2_number = ((word >> 20) & 0x1F);
  imm = ((word >> 20) & 0xFE0) | ((word >> 7) & 0x1F);

  imm = extendBit(imm, 11);
}

void CPU::getBTypeArgs(u32 word, u32 &rs1_number, u32 &rs2_number, u32 &imm)
{
  rs1_number = ((word >> 15) & 0x1F);
  rs2_number = ((word >> 20) & 0x1F);
  imm = ((word >> 19) & 0x1000) | ((word << 4) & 0x800) | ((word >> 20) & 0x7E0) | ((word >> 7) & 0x1E);

  imm = extendBit(imm, 12);
}

/*
 * This function extends the given bit to the left
 * Works by ORing the given bit to each bit to the left, so might not work if given bit is not the left most set bit
 * Used for sign extension
 */
u32 CPU::extendBit(u32 word, u8 bit)
{
  assert(bit < 32 && "signExtendBit: bit too high");

  u32 extended_word = word;

  for(int i = 1; i < (32 - bit); i++)
  {
    extended_word |= ((word & (0x1 << bit)) << i);
  }

  return extended_word;
}

/*
 * The following functions implement the instructions
 * Depending on the instruction type, they read the necessary arguments and execute the instruction
 * They do not write anything to reg[0] as this register is read-only
 * returns true when pc should be incremented
 * returns false when pc has been changed and should not be incremented
 */
bool CPU::ADD(u32 word)
{
  u32 rd_number, rs1_number, rs2_number;
  getRTypeArgs(word, rd_number, rs1_number, rs2_number);

  if(rd_number != 0)
  {
    registers_[rd_number] = registers_[rs1_number] + registers_[rs2_number];
  }

  return true;
}

bool CPU::ADDI(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
    registers_[rd_number] = registers_[rs1_number] + imm;
  }

  return true;
}

bool CPU::AND(u32 word)
{
  u32 rd_number, rs1_number, rs2_number;
  getRTypeArgs(word, rd_number, rs1_number, rs2_number);

  if(rd_number != 0)
  {
    registers_[rd_number] = registers_[rs1_number] & registers_[rs2_number];
  }

  return true;
}

bool CPU::ANDI(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
    registers_[rd_number] = registers_[rs1_number] & imm;
  }

  return true;
}

bool CPU::AUIPC(u32 word)
{
  u32 rd_number, imm;
  getUTypeArgs(word, rd_number, imm);

  if(rd_number != 0)
  {
    registers_[rd_number] = pc_ + imm;
  }

  return true;
}

bool CPU::BEQ(u32 word)
{
  u32 rs1_number, rs2_number, imm;
  getBTypeArgs(word, rs1_number, rs2_number, imm);

  if(registers_[rs1_number] == registers_[rs2_number])
  {
    pc_ += imm;
    return false;
  }

  return true;
}

bool CPU::BGE(u32 word)
{
  u32 rs1_number, rs2_number, imm;
  getBTypeArgs(word, rs1_number, rs2_number, imm);

  if((s32) registers_[rs1_number] >= (s32) registers_[rs2_number])
  {
    pc_ += imm;
    return false;
  }

  return true;
}

bool CPU::BGEU(u32 word)
{
  u32 rs1_number, rs2_number, imm;
  getBTypeArgs(word, rs1_number, rs2_number, imm);

  if(registers_[rs1_number] == registers_[rs2_number])
  {
    pc_ += imm;
    return false;
  }

  return true;
}

bool CPU::BLT(u32 word)
{
  u32 rs1_number, rs2_number, imm;
  getBTypeArgs(word, rs1_number, rs2_number, imm);

  if((s32) registers_[rs1_number] < (s32) registers_[rs2_number])
  {
    pc_ += imm;
    return false;
  }

  return true;
}

bool CPU::BLTU(u32 word)
{
  u32 rs1_number, rs2_number, imm;
  getBTypeArgs(word, rs1_number, rs2_number, imm);

  if(registers_[rs1_number] < registers_[rs2_number])
  {
    pc_ += imm;
    return false;
  }

  return true;
}

bool CPU::BNE(u32 word)
{
  u32 rs1_number, rs2_number, imm;
  getBTypeArgs(word, rs1_number, rs2_number, imm);

  if(registers_[rs1_number] != registers_[rs2_number])
  {
    pc_ += imm;
    return false;
  }

  return true;
}

bool CPU::EBREAK(u32 word)
{
  exit(0);
}

bool CPU::JAL(u32 word)
{
  u32 rd_number, imm;
  getJTypeArgs(word, rd_number, imm);

  if(rd_number != 0)
  {
    registers_[rd_number] = pc_ + 4;
  }

  pc_ += imm;
  return false;
}

bool CPU::JALR(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
    registers_[rd_number] = pc_ + 4;
  }

  pc_ = (registers_[rs1_number] + imm) & (~1);
  return false;
}

bool CPU::LB(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
    registers_[rd_number] = extendBit(read(registers_[rs1_number] + imm) & 0xFF, 7);
  }

  return true;
}

bool CPU::LBU(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
    registers_[rd_number] = read(registers_[rs1_number] + imm) & 0xFF;
  }

  return true;
}

bool CPU::LH(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
    u32 read_halfword = read(registers_[rs1_number] + imm) | (read(registers_[rs1_number] + imm + 1) << 8);
    registers_[rd_number] = extendBit(read_halfword, 15);
  }

  return true;
}

bool CPU::LHU(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
    u32 read_halfword = read(registers_[rs1_number] + imm) | (read(registers_[rs1_number] + imm + 1) << 8);
    registers_[rd_number] = read_halfword;
  }

  return true;
}

bool CPU::LUI(u32 word)
{
  u32 rd_number, imm;
  getUTypeArgs(word, rd_number, imm);

  if(rd_number != 0)
  {
    registers_[rd_number] = imm;
  }

  return true;
}

bool CPU::LW(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
    u32 read_word = read(registers_[rs1_number] + imm) | (read(registers_[rs1_number] + imm + 1) << 8) |
      (read(registers_[rs1_number] + imm + 2) << 16) | (read(registers_[rs1_number] + imm + 3) << 24);
    registers_[rd_number] = read_word;
  }

  return true;
}

bool CPU::OR(u32 word)
{
  u32 rd_number, rs1_number, rs2_number;
  getRTypeArgs(word, rd_number, rs1_number, rs2_number);

  if(rd_number != 0)
  {
    registers_[rd_number] = registers_[rs1_number] | registers_[rs2_number];
  }

  return true;
}

bool CPU::ORI(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
    registers_[rd_number] = registers_[rs1_number] | imm;
  }

  return true;
}

bool CPU::SB(u32 word)
{
  u32 rs1_number, rs2_number, imm;
  getSTypeArgs(word, rs1_number, rs2_number, imm);

  write(registers_[rs1_number] + imm, (u8) registers_[rs2_number]);
  return true;
}

bool CPU::SH(u32 word)
{
  u32 rs1_number, rs2_number, imm;
  getSTypeArgs(word, rs1_number, rs2_number, imm);

  write(registers_[rs1_number] + imm, (u8) registers_[rs2_number]);
  write(registers_[rs1_number] + imm + 1, (u8) (registers_[rs2_number] >> 8));
  return true;
}

bool CPU::SLL(u32 word)
{
  u32 rd_number, rs1_number, rs2_number;
  getRTypeArgs(word, rd_number, rs1_number, rs2_number);

  if(rd_number != 0)
  {
    registers_[rd_number] = registers_[rs1_number] << (registers_[rs2_number] % 32);
  }

  return true;
}

bool CPU::SLLI(u32 word)
{
  u32 rd_number, rs1_number, shamt;
  getI2TypeArgs(word, rd_number, rs1_number, shamt);

  if(rd_number != 0)
  {
    registers_[rd_number] = registers_[rs1_number] << shamt;
  }

  return true;
}

bool CPU::SLT(u32 word)
{
  u32 rd_number, rs1_number, rs2_number;
  getRTypeArgs(word, rd_number, rs1_number, rs2_number);

  if(rd_number != 0)
  {
    registers_[rd_number] = ((s32) registers_[rs1_number] < (s32) registers_[rs2_number]) ? 1 : 0;
  }

  return true;
}

bool CPU::SLTI(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
    registers_[rd_number] = ((s32) registers_[rs1_number] < (s32) imm) ? 1 : 0;
  }

  return true;
}

bool CPU::SLTIU(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
    registers_[rd_number] = (registers_[rs1_number] < imm) ? 1 : 0;
  }

  return true;
}

bool CPU::SLTU(u32 word)
{
  u32 rd_number, rs1_number, rs2_number;
  getRTypeArgs(word, rd_number, rs1_number, rs2_number);

  if(rd_number != 0)
  {
    registers_[rd_number] = (registers_[rs1_number] < registers_[rs2_number]) ? 1 : 0;
  }

  return true;
}

bool CPU::SRA(u32 word)
{
  u32 rd_number, rs1_number, rs2_number;
  getRTypeArgs(word, rd_number, rs1_number, rs2_number);

  if(rd_number != 0)
  {
    u32 shift = registers_[rs2_number] % 32;
    registers_[rd_number] = extendBit(registers_[rs1_number] >> shift, (31 - shift));
  }

  return true;
}

bool CPU::SRAI(u32 word)
{
  u32 rd_number, rs1_number, shamt;
  getI2TypeArgs(word, rd_number, rs1_number, shamt);

  if(rd_number != 0)
  {
    registers_[rd_number] = extendBit(registers_[rs1_number] >> shamt, (31 - shamt));
  }

  return true;
}

bool CPU::SRL(u32 word)
{
  u32 rd_number, rs1_number, rs2_number;
  getRTypeArgs(word, rd_number, rs1_number, rs2_number);
  u32 shift = registers_[rs2_number] % 32;

  if(rd_number != 0)
  {
    registers_[rd_number] = registers_[rs1_number] >> shift;
  }

  return true;
}

bool CPU::SRLI(u32 word)
{
  u32 rd_number, rs1_number, shamt;
  getI2TypeArgs(word, rd_number, rs1_number, shamt);

  if(rd_number != 0)
  {
    registers_[rd_number] = registers_[rs1_number] >> shamt;
  }

  return true;
}

bool CPU::SUB(u32 word)
{
  u32 rd_number, rs1_number, rs2_number;
  getRTypeArgs(word, rd_number, rs1_number, rs2_number);

  if(rd_number != 0)
  {
    registers_[rd_number] = registers_[rs1_number] - registers_[rs2_number];
  }

  return true;
}

bool CPU::SW(u32 word)
{
  u32 rs1_number, rs2_number, imm;
  getSTypeArgs(word, rs1_number, rs2_number, imm);

  write(registers_[rs1_number] + imm, (u8) registers_[rs2_number]);
  write(registers_[rs1_number] + imm + 1, (u8) (registers_[rs2_number] >> 8));
  write(registers_[rs1_number] + imm + 2, (u8) (registers_[rs2_number] >> 16));
  write(registers_[rs1_number] + imm + 3, (u8) (registers_[rs2_number] >> 24));
  return true;
}

bool CPU::XOR(u32 word)
{
  u32 rd_number, rs1_number, rs2_number;
  getRTypeArgs(word, rd_number, rs1_number, rs2_number);

  if(rd_number != 0)
  {
    registers_[rd_number] = registers_[rs1_number] ^ registers_[rs2_number];
  }

  return true;
}

bool CPU::XORI(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
    registers_[rd_number] = registers_[rs1_number] ^ imm;
  }

  return true;
}

