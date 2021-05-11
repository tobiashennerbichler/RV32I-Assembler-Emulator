#include <iostream>
#include "../include/Interpreter.h"
#include "../include/TestSystem.h"

Interpreter::Interpreter() : memory_(0x1000), registers_(32)
{

}

void Interpreter::run()
{
  //TestSystem::testFiles();
  std::vector<Instruction> instructions;
  Parser::instance()->parse(instructions, "../RISC-V-Interpreter/FileTests/test0.txt");

  //TODO: where do we start? at _start label? at main label? implement pc handling
  for(auto &instruction : instructions)
  {
    printf("execute %s\n", instruction.name_.c_str());
    executeInstruction(instruction);
  }
}

void Interpreter::ADD(u32 &Rd, u32 Rs1, u32 Rs2)
{
  Rd = Rs1 + Rs2;
  printf("add %d and %d to %d\n", Rd, Rs1, Rs2);
}

void Interpreter::ADDI(u32 &Rd, u32 Rs1, u32 imm)
{
  Rd = Rs1 + imm;
}

void Interpreter::SUB(u32 &Rd, u32 Rs1, u32 Rs2)
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

void Interpreter::executeInstruction(Instruction &instruction)
{
  if(instruction.name_ == "ADD")
  {
    ADD(registers_.at(instruction.Rd_), registers_.at(instruction.Rs1_), registers_.at(instruction.Rs2_));
  }
  else if(instruction.name_ == "ADDI")
  {
    ADDI(registers_.at(instruction.Rd_), registers_.at(instruction.Rs1_), instruction.imm_);
  }
  else if(instruction.name_ == "SUB")
  {
    SUB(registers_.at(instruction.Rd_), registers_.at(instruction.Rs1_), registers_.at(instruction.Rs2_));
  }
  else if(instruction.name_ == "LW")
  {
    LW(registers_.at(instruction.Rd_), registers_.at(instruction.Rs1_), instruction.imm_);
  }
  else if(instruction.name_ == "SW")
  {
    SW(registers_.at(instruction.Rs1_), registers_.at(instruction.Rs2_), instruction.imm_);
  }
  else if(instruction.name_ == "JAL")
  {
    JAL(registers_.at(instruction.Rd_), instruction.imm_);
  }
  else if(instruction.name_ == "JALR")
  {
    JALR(registers_.at(instruction.Rd_), registers_.at(instruction.Rs1_), instruction.imm_);
  }
  else if(instruction.name_ == "BEQ")
  {
    BEQ(registers_.at(instruction.Rs1_), registers_.at(instruction.Rs2_), instruction.imm_);
  }
  else if(instruction.name_ == "BNE")
  {
    BNE(registers_.at(instruction.Rs1_), registers_.at(instruction.Rs2_), instruction.imm_);
  }
  else if(instruction.name_ == "BLT")
  {
    BLT(registers_.at(instruction.Rs1_), registers_.at(instruction.Rs2_), instruction.imm_);
  }
  else if(instruction.name_ == "BGE")
  {
    BGE(registers_.at(instruction.Rs1_), registers_.at(instruction.Rs2_), instruction.imm_);
  }
  else if(instruction.name_ == "EBREAK")
  {
    EBREAK();
  }
}
