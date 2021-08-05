#include "../include/Assembler.h"
#include <fstream>

/*
 * This function uses the parser to parse the given file and then assembles each instruction
 * into its corresponding machine word and writes into a binary file
 * Also replaces the labels in the J(ump) and B(ranch) type instructions by their relative addresses to the current pc
 */
void Assembler::assemble(std::string file_name)
{
  std::vector<Instruction> instructions;
  parser_.parse(instructions, file_name);

  std::fstream file = std::fstream("../memory.bin", std::ios::out | std::ios::binary);

  if(!file.is_open())
  {
    printf("file not open\n");
    exit(-1);
  }

  assignLabelAddress(instructions);

  for(auto &instruction : instructions)
  {
    u32 machine_word = getMachineWord(instruction);

    for(int i = 0; i < 4; i++)
    {
      u8 byte = (u8) machine_word;
      file.write((char *) &byte, sizeof(u8));
      machine_word >>= 8;
    }
  }

  for(auto &label : parser_.label_lut_)
  {
    printf("address of %s: 0x%x\n", label.first.c_str(), label.second.address_);
  }
}

/*
 * Calculates the relative address of the used labels to the current pc and replaces them by it
 */
void Assembler::assignLabelAddress(std::vector<Instruction> &instructions)
{
  u32 addr = 0;
  //search for use of label and replace it with address
  for(auto &instruction : instructions)
  {
    if(instruction.label_name_ == "")
    {
      addr += 4;
      continue;
    }

    auto label = parser_.label_lut_.find(instruction.label_name_);

    if(label == parser_.label_lut_.end())
    {
      printf("Invalid label %s\n", instruction.label_name_.c_str());
      exit(-1);
    }

    instruction.imm_ = label->second.address_ - addr;
    addr += 4;
  }
}

/*
 * Assembles the given instruction based on its instruction type
 */
u32 Assembler::getMachineWord(Instruction &instruction)
{
  u32 hex = 0;
  InstructionInfo instruction_info = instruction.info_;
  hex |= (instruction_info.opcode_ & OPCODE_MASK);

  switch(instruction_info.type_)
  {
    case U_TYPE:
      UType(hex, instruction);
      break;
    case J_TYPE:
      JType(hex, instruction);
      break;
    case R_TYPE:
      RType(hex, instruction);
      break;
    case I_TYPE:
      IType(hex, instruction);
      break;
    case I_TYPE2:
      I2Type(hex, instruction);
      break;
    case S_TYPE:
      SType(hex, instruction);
      break;
    case B_TYPE:
      BType(hex, instruction);
      break;
  }

  return hex;
}

/*
 * The following functions create the machine word according to the pattern of the given instruction type
 */
void Assembler::UType(u32 &hex, Instruction &instruction)
{
  hex |= (((u32) instruction.Rd_ << 7) & 0xF80);
  hex |= ((u32) instruction.imm_ & 0xFFFFF000);
}

void Assembler::JType(u32 &hex, Instruction &instruction)
{
  hex |= (((u32) instruction.Rd_ << 7) & 0xF80);
  u32 imm1912 = instruction.imm_ & 0xFF000;
  u32 imm11 = instruction.imm_ & 0x800;
  u32 imm101 = instruction.imm_ & 0x7FE;
  u32 imm20 = instruction.imm_ & 0x100000;

  hex |= (imm20 << 11) | (imm101 << 20) | (imm11 << 9) | imm1912;
}

void Assembler::RType(u32 &hex, Instruction &instruction)
{
  InstructionInfo instruction_info = instruction.info_;

  hex |= (((u32) instruction.Rd_ << 7) & 0xF80);
  hex |= ((instruction_info.func3_ << 12) & 0x7000);
  hex |= (((u32) instruction.Rs1_ << 15) & 0xF8000);
  hex |= (((u32) instruction.Rs2_ << 20) & 0x1F00000);
  hex |= ((instruction_info.func7_ << 25) & 0xFE000000);
}

void Assembler::IType(u32 &hex, Instruction &instruction)
{
  InstructionInfo instruction_info = instruction.info_;

  hex |= (((u32) instruction.Rd_ << 7) & 0xF80);
  hex |= ((instruction_info.func3_ << 12) & 0x7000);
  hex |= (((u32) instruction.Rs1_ << 15) & 0xF8000);
  hex |= (((u32) instruction.imm_ << 20) & 0xFFF00000);
}

void Assembler::I2Type(u32 &hex, Instruction &instruction)
{
  InstructionInfo instruction_info = instruction.info_;

  hex |= (((u32) instruction.Rd_ << 7) & 0xF80);
  hex |= ((instruction_info.func3_ << 12) & 0x7000);
  hex |= (((u32) instruction.Rs1_ << 15) & 0xF8000);
  hex |= (((u32) instruction.imm_ << 20) & 0x1F00000);
  hex |= ((instruction_info.func7_ << 25) & 0xFE000000);
}

void Assembler::SType(u32 &hex, Instruction &instruction)
{
  InstructionInfo instruction_info = instruction.info_;

  hex |= (((u32) instruction.imm_ << 7) & 0xF80);
  hex |= ((instruction_info.func3_ << 12) & 0x7000);
  hex |= (((u32) instruction.Rs1_ << 15) & 0xF8000);
  hex |= (((u32) instruction.Rs2_ << 20) & 0x1F00000);
  hex |= (((u32) instruction.imm_ << 14) & 0xFE000000);
}

void Assembler::BType(u32 &hex, Instruction &instruction)
{
  InstructionInfo instruction_info = instruction.info_;

  u32 imm11 = (instruction.imm_ >> 4) & 0x80;
  u32 imm41 = (instruction.imm_ << 7) & 0xF00;
  u32 imm105 = (instruction.imm_ << 20) & 0x7E000000;
  u32 imm12 = (instruction.imm_ << 19) & 0x80000000;
  hex |= imm11 | imm41;
  hex |= ((instruction_info.func3_ << 12) & 0x7000);
  hex |= (((u32) instruction.Rs1_ << 15) & 0xF8000);
  hex |= (((u32) instruction.Rs2_ << 20) & 0x1F00000);
  hex |= imm105 | imm12;
}
