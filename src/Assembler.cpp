#include "../include/Assembler.h"

Assembler::Assembler(CPU &cpu, std::string &filename) : cpu_(cpu), file_name_(filename)
{
  opcodes_ = {{"EBREAK", 0x0}, {"ADD", 0x1}, {"ADDI", 0x2}, {"SUB", 0x3}, {"SW", 0x4},
              {"LW", 0x5}, {"JAL", 0x6}, {"JALR", 0x7}, {"BEQ", 0x8},
              {"BNE", 0x9}, {"BLT", 0xA}, {"BGE", 0xB}};

  assemble();
}

Assembler::~Assembler()
{

}

/*
 * Transforms instructions into machine code and writes it into memory for CPU to execute
 */
void Assembler::assemble()
{
  std::vector<Instruction> instructions;
  parser_.parse(instructions, file_name_);
  u32 addr = 0;

  assignLabelAddress(instructions);

  for(auto &instruction : instructions)
  {
    if(instruction.label_)
    {
      continue;
    }

    u32 machine_code = getHexRepresentation(instruction);
    cpu_.write(addr, machine_code);
    addr += 4;
  }

  for(u32 address = 0; address < addr; address += 4)
  {
    printf("0x%x\n", cpu_.read(address));
  }
}

/*
 * Goes through list of instructions and assigns addresses to labels according to next instruction used
 * Sets imm values correctly for instructions that use these labels
 * param: instructions of the program
 */
void Assembler::assignLabelAddress(std::vector<Instruction> &instructions)
{
  u32 addr = 0;

  //give each label an address
  for(auto &instruction : instructions)
  {
    if(instruction.label_)
    {
      std::string label_name = instruction.name_.substr(0, instruction.name_.size() - 1);
      auto label = parser_.label_lut_.find(label_name);

      if(label == parser_.label_lut_.end())
      {
        printf("label %s not stored\n", label_name.c_str());
        exit(-1);
      }

      label->second.address_ = addr;
    }
    else
    {
      addr += 4;
    }
  }

  //search for use of label and replace it with address
  for(auto &instruction : instructions)
  {
    if(instruction.label_ || instruction.label_name_ == "")
    {
      continue;
    }

    auto label = parser_.label_lut_.find(instruction.label_name_);

    if(label == parser_.label_lut_.end())
    {
      printf("Invalid label %s\n", instruction.label_name_.c_str());
      exit(-1);
    }

    instruction.imm_ = parser_.label_lut_.find(instruction.label_name_)->second.address_;
  }
}

/*
 * puts instruction into right format:
 *
 *  funct7   rs2   rs1  funct3 rd  opcode
 * |-------|-----|-----|---|-----|-------| <-- R-type (ADD, SUB)
 *   imm12        rs1  funct3 rd  opcode
 * |------------|-----|---|-----|-------| <-- I-type (LW, JALR, ADDI)
 *    imm7   rs2   rs1  funct3 imm5 opcode
 * |-------|-----|-----|---|-----|-------| <-- S-type (SW, BEQ, BNE, BLT, BGE)
 *           imm20         rd   opcode
 * |--------------------|-----|-------| <-- U-type (JAL)
 *
 * param: instruction to transform
 * return: hex representation of instruction
 */
u32 Assembler::getHexRepresentation(Instruction &instruction)
{
  u32 hex = 0;
  u32 opcode = opcodes_.find(instruction.name_)->second;
  hex |= (opcode & OPCODE_MASK);

  if(instruction.name_ == "ADD" || instruction.name_ == "SUB")
  {
    hex |= (((u32) instruction.Rd_ << RD_SHIFT) & RD_MASK);
    hex |= (((u32) instruction.Rs1_ << RS1_SHIFT) & RS1_MASK);
    hex |= (((u32) instruction.Rs2_ << RS2_SHIFT) & RS2_MASK);
  }
  else if(instruction.name_ == "LW" || instruction.name_ == "JALR" || instruction.name_ == "ADDI")
  {
    hex |= (((u32) instruction.Rd_ << RD_SHIFT) & RD_MASK);
    hex |= (((u32) instruction.Rs1_ << RS1_SHIFT) & RS1_MASK);
    hex |= (((u32) instruction.imm_ << IMM12_SHIFT) & IMM12_MASK);
  }
  //TODO: somehow make signed values work, read how they are provided
  else if(instruction.name_ == "SW")
  {
    hex |= (((u32) instruction.imm_ << IMM5_SHIFT) & IMM5_MASK);
    hex |= (((u32) instruction.Rs1_ << RS1_SHIFT) & RS1_MASK);
    hex |= (((u32) instruction.Rs2_ << RS2_SHIFT) & RS2_MASK);
    hex |= ((((u32) instruction.imm_ >> IMM5_SHIFT) << IMM7_SHIFT) & IMM7_MASK);
  }
  else if(instruction.name_ == "JAL")
  {
    hex |= (((u32) instruction.Rd_ << RD_SHIFT) & RD_MASK);
    hex |= (((u32) instruction.imm_ << IMM20_SHIFT) & IMM20_MASK);
  }
  else if(instruction.name_ == "BEQ" || instruction.name_ == "BNE" || instruction.name_ == "BLT" ||
    instruction.name_ == "BGE")
  {
    hex |= (((u32) instruction.imm_ << IMM5_SHIFT) & IMM5_MASK);
    hex |= (((u32) instruction.Rs1_ << RS1_SHIFT) & RS1_MASK);
    hex |= (((u32) instruction.Rs2_ << RS2_SHIFT) & RS2_MASK);
    hex |= ((((u32) instruction.imm_ >> IMM5_SHIFT) << IMM7_SHIFT) & IMM7_MASK);
  }

  return hex;
}
