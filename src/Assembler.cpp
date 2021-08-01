#include "../include/Assembler.h"
#include <fstream>

Assembler::Assembler(CPU &cpu) : cpu_(cpu)
{
  //TODO: make EBREAK I_TYPE and set imm so the 20th bit is 1
  opcodes_ = {{"EBREAK", {0x73, 0x00, 0x00, NONE}},
              {"LUI", {0x37, 0x00, 0x00, U_TYPE}},
              {"AUIPC", {0x37, 0x00, 0x00, U_TYPE}},
              {"JAL", {0x6F, 0x00, 0x00, J_TYPE}},
              {"JALR", {0x67, 0x00, 0x00, I_TYPE}},
              {"BEQ", {0x63, 0x00, 0x00, B_TYPE}},
              {"BNE", {0x63, 0x01, 0x00, B_TYPE}},
              {"BLT", {0x63, 0x04, 0x00, B_TYPE}},
              {"BGE", {0x63, 0x05, 0x00, B_TYPE}},
              {"BLTU", {0x63, 0x06, 0x00, B_TYPE}},
              {"BGEU", {0x63, 0x07, 0x00, B_TYPE}},
              {"LB", {0x03, 0x00, 0x00, I_TYPE}},
              {"LH", {0x03, 0x01, 0x00, I_TYPE}},
              {"LW", {0x03, 0x02, 0x00, I_TYPE}},
              {"LBU", {0x03, 0x04, 0x00, I_TYPE}},
              {"LHU", {0x03, 0x05, 0x00, I_TYPE}},
              {"ADDI", {0x13, 0x00, 0x00, I_TYPE}}};
}

Assembler::~Assembler()
{

}

/*
 * Transforms instructions into machine code and writes it into memory for CPU to execute
 */
void Assembler::assemble(std::string file_name)
{
  std::vector<Instruction> instructions;
  parser_.parse(instructions, file_name);
  u32 addr = 0;

  assignLabelAddress(instructions);

  for(auto &instruction : instructions)
  {
    u32 machine_code = getHexRepresentation(instruction);
    printf("hex: 0x%x\n", machine_code);

    for(int i = 0; i < 4; i++)
    {
      cpu_.write(addr, (u8) machine_code);
      addr++;
      machine_code >>= 8;
    }
  }

  std::fstream file = std::fstream("../memory.bin", std::ios::out | std::ios::binary);

  if(!file.is_open())
  {
    printf("file not open\n");
    exit(-1);
  }

  for(u32 address = 0; address < addr; address++)
  {
    u8 byte = cpu_.read(address);
    file.write((char *) &byte, sizeof(u8));
  }

  file.flush();

  for(auto &label : parser_.label_lut_)
  {
    printf("address of %s: 0x%x\n", label.first.c_str(), label.second.address_);
  }
}

/*
 * Goes through list of instructions and assigns addresses to labels according to next instruction used
 * Sets imm values correctly for instructions that use these labels
 * param: instructions of the program
 */
void Assembler::assignLabelAddress(std::vector<Instruction> &instructions)
{
  //search for use of label and replace it with address
  for(auto &instruction : instructions)
  {
    if(instruction.label_name_ == "")
    {
      continue;
    }

    auto label = parser_.label_lut_.find(instruction.label_name_);

    if(label == parser_.label_lut_.end())
    {
      printf("Invalid label %s\n", instruction.label_name_.c_str());
      exit(-1);
    }

    instruction.imm_ = label->second.address_;
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
  //TODO: get the instruction info into instruction in parsing?
  auto instruction_info = opcodes_.find(instruction.name_);
  hex |= (instruction_info->second.opcode_ & OPCODE_MASK);

  //TODO: make function pointer
  if(instruction_info->second.type_ == U_TYPE)
  {
    UType(hex, instruction);
  }
  else if(instruction_info->second.type_ == J_TYPE)
  {
    JType(hex, instruction);
  }
  else if(instruction_info->second.type_ == R_TYPE)
  {
    RType(hex, instruction);
  }
  else if(instruction_info->second.type_ == I_TYPE)
  {
    IType(hex, instruction);
  }
  else if(instruction_info->second.type_ == I_TYPE2)
  {
    I2Type(hex, instruction);
  }
  else if(instruction_info->second.type_ == S_TYPE)
  {
    SType(hex, instruction);
  }
  else if(instruction_info->second.type_ == B_TYPE)
  {
    BType(hex, instruction);
  }

  return hex;
}

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
  InstructionInfo instruction_info = opcodes_.find(instruction.name_)->second;

  hex |= (((u32) instruction.Rd_ << 7) & 0xF80);
  hex |= ((instruction_info.func3_ << 12) & 0x7000);
  hex |= (((u32) instruction.Rs1_ << 15) & 0xF8000);
  hex |= (((u32) instruction.Rs2_ << 20) & 0x1F00000);
  hex |= ((instruction_info.func7_ << 25) & 0xFE000000);
}

void Assembler::IType(u32 &hex, Instruction &instruction)
{
  InstructionInfo instruction_info = opcodes_.find(instruction.name_)->second;

  hex |= (((u32) instruction.Rd_ << 7) & 0xF80);
  hex |= ((instruction_info.func3_ << 12) & 0x7000);
  hex |= (((u32) instruction.Rs1_ << 15) & 0xF8000);
  hex |= (((u32) instruction.imm_ << 20) & 0xFFF00000);
}

void Assembler::I2Type(u32 &hex, Instruction &instruction)
{
  InstructionInfo instruction_info = opcodes_.find(instruction.name_)->second;

  hex |= (((u32) instruction.Rd_ << 7) & 0xF80);
  hex |= ((instruction_info.func3_ << 12) & 0x7000);
  hex |= (((u32) instruction.Rs1_ << 15) & 0xF8000);
  //TODO: shamt?
  hex |= ((instruction_info.func7_ << 25) & 0xFE000000);
}

void Assembler::SType(u32 &hex, Instruction &instruction)
{
  InstructionInfo instruction_info = opcodes_.find(instruction.name_)->second;

  hex |= (((u32) instruction.imm_ << 7) & 0xF80);
  hex |= ((instruction_info.func3_ << 12) & 0x7000);
  hex |= (((u32) instruction.Rs1_ << 15) & 0xF8000);
  hex |= (((u32) instruction.Rs2_ << 20) & 0x1F00000);
  hex |= (((u32) instruction.imm_ << 14) & 0xFE000000);
}

void Assembler::BType(u32 &hex, Instruction &instruction)
{
  InstructionInfo instruction_info = opcodes_.find(instruction.name_)->second;

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
