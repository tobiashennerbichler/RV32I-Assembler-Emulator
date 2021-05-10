#include <iostream>
#include "../include/Interpreter.h"
#include "../include/TestSystem.h"

Interpreter::Interpreter() : memory_(0x1000), registers_(32)
{

}

void Interpreter::run()
{
  TestSystem::testFiles();
}

void Interpreter::ADD(s32 &Rd, s32 Rs1, s32 Rs2)
{
  Rd = Rs1 + Rs2;
}

void Interpreter::ADDI(s32 &Rd, s32 Rs1, s32 imm)
{
  Rd = Rs1 + imm;
}

void Interpreter::SUB(s32 &Rd, s32 Rs1, s32 Rs2)
{
  Rd = Rs1 - Rs2;
}

void Interpreter::LW(u32 &Rd, u32 Rs1, u32 imm)
{
  Rd = memory_.at(Rs1 + imm);
}

void Interpreter::SW(u32 Rs1, u32 Rs2, u32 imm)
{
  memory_.at(Rs2 + imm) = Rs1;
}

void Interpreter::JAL(u32 &Rd, u32 imm)
{
  Rd = pc_ + 4;
  pc_ = imm;
}

void Interpreter::JALR(u32 Rd, u32 Rs1, u32 imm)
{
  pc_ = memory_.at(Rs1 + imm);
}

void Interpreter::BEQ(u32 Rs1, u32 Rs2, u32 imm)
{
  if(Rs1 == Rs2)
  {
    pc_ = imm;
  }
}

void Interpreter::BNE(u32 Rs1, u32 Rs2, u32 imm)
{
  if(Rs1 != Rs2)
  {
    pc_ = imm;
  }
}

void Interpreter::BLT(u32 Rs1, u32 Rs2, u32 imm)
{
  if(Rs1 < Rs2)
  {
    pc_ = imm;
  }
}

void Interpreter::BGE(u32 Rs1, u32 Rs2, u32 imm)
{
  if(Rs1 >= Rs2)
  {
    pc_ = imm;
  }
}

[[noreturn]] void Interpreter::EBREAK()
{
  while(1)
  {

  }
}
