#include "../include/CPU.h"

CPU::CPU() : memory_(0x10000)
{
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

  //U_TYPE: 0x37, 0x17
  //J_TYPE: 0x6F
  //R_TYPE: 0x33
  //I_TYPE: 0x67, 0x03, 0x13, 0x73
  //I_TYPE2: 0x13
  //S_TYPE: 0x23
  //B_TYPE: 0x63
}

CPU::~CPU()
{

}

u8 CPU::read(u32 address)
{
  return memory_.at(address);
}

void CPU::write(u32 address, u8 data)
{
  memory_.at(address) = data;
}

void CPU::ADD(u32 word)
{

}

void CPU::ADDI(u32 word)
{

}

void CPU::AND(u32 word)
{

}

void CPU::ANDI(u32 word)
{

}

void CPU::AUIPC(u32 word)
{

}

void CPU::BEQ(u32 word)
{

}

void CPU::BGE(u32 word)
{

}

void CPU::BGEU(u32 word)
{

}

void CPU::BLT(u32 word)
{

}

void CPU::BLTU(u32 word)
{

}

void CPU::BNE(u32 word)
{

}

void CPU::EBREAK(u32 word)
{

}

void CPU::JAL(u32 word)
{

}

void CPU::JALR(u32 word)
{

}

void CPU::LB(u32 word)
{

}

void CPU::LBU(u32 word)
{

}

void CPU::LH(u32 word)
{

}

void CPU::LHU(u32 word)
{

}

void CPU::LUI(u32 word)
{

}

void CPU::LW(u32 word)
{

}

void CPU::OR(u32 word)
{

}

void CPU::ORI(u32 word)
{

}

void CPU::SB(u32 word)
{

}

void CPU::SH(u32 word)
{

}

void CPU::SLL(u32 word)
{

}

void CPU::SLLI(u32 word)
{

}

void CPU::SLT(u32 word)
{

}

void CPU::SLTI(u32 word)
{

}

void CPU::SLTIU(u32 word)
{

}

void CPU::SLTU(u32 word)
{

}

void CPU::SRA(u32 word)
{

}

void CPU::SRAI(u32 word)
{

}

void CPU::SRL(u32 word)
{

}

void CPU::SRLI(u32 word)
{

}

void CPU::SUB(u32 word)
{

}

void CPU::SW(u32 word)
{

}

void CPU::XOR(u32 word)
{

}

void CPU::XORI(u32 word)
{

}

