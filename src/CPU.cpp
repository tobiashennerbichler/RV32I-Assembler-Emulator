#include <cassert>
#include "../include/CPU.h"

CPU::CPU() : memory_size_(0x10000), memory_(memory_size_), pc_(0)
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
  //TODO: maybe not assert but exit
  assert(address < memory_size_ && "Out of range");

  return memory_.at(address);
}

/*
 * Write one byte to the given address in memory
 */
void CPU::write(u32 address, u8 data)
{
  assert(address < memory_size_ && "Out of range");

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

#ifdef DEBUG
  printf("0x%x: ", pc_);
#endif

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
  //Rd number stored as Bit 11-7
  rd_number = (word >> 7) & 0x1F;
  //Imm
  //Bit 31-12 of imm stored as Bit 31-12 in word, remove everything else
  //Bit 11-0 of imm = 0
  imm = word & 0xFFFFF000;
}

void CPU::getJTypeArgs(u32 word, u32 &rd_number, u32 &imm)
{
  //Rd number stored as Bit 11-7
  rd_number = (word >> 7) & 0x1F;
  //Imm
  //Bit 20 of imm stored as Bit 31 in word --> right shift by 11 and remove everything else
  //Bit 19-12 of imm stored as Bit 19-12 in word, remove everything else
  //Bit 11 of imm stored as Bit 20 in word --> right shift by 9 and remove everything else
  //Bit 10-1 of imm stored as Bit 30-21 in word --> right shift by 20 and remove everything else
  //Bit 0 of imm = 0
  imm = ((word >> 11) & 0x100000) | (word & 0xFF000) | ((word >> 9) & 0x800) | ((word >> 20) & 0x7FE);

  imm = extendBit(imm, 20);
}

void CPU::getRTypeArgs(u32 word, u32 &rd_number, u32 &rs1_number, u32 &rs2_number)
{
  //Rd number stored as Bit 11-7
  rd_number = ((word >> 7) & 0x1F);
  //Rs1 number stored as Bit 19-15
  rs1_number = ((word >> 15) & 0x1F);
  //Rs2 number stored as Bit 24-20
  rs2_number = ((word >> 20) & 0x1F);
}

void CPU::getITypeArgs(u32 word, u32 &rd_number, u32 &rs1_number, u32 &imm)
{
  //Rd number stored as Bit 11-7
  rd_number = ((word >> 7) & 0x1F);
  //Rs1 number stored as Bit 19-15
  rs1_number = ((word >> 15) & 0x1F);
  //Imm
  //Bit 11-0 of imm stored as Bit 31-20 in word --> right shift by 20 and remove everything else
  //Sign extend Bit 11 of imm to the left
  imm = ((word >> 20) & 0xFFF);

  imm = extendBit(imm, 11);
}

void CPU::getI2TypeArgs(u32 word, u32 &rd_number, u32 &rs1_number, u32 &shamt)
{
  //Rd number stored as Bit 11-7
  rd_number = ((word >> 7) & 0x1F);
  //Rs1 number stored as Bit 19-15
  rs1_number = ((word >> 15) & 0x1F);
  //Shamt stored as Bit 24-20
  shamt = ((word >> 20) & 0x1F);
}

void CPU::getSTypeArgs(u32 word, u32 &rs1_number, u32 &rs2_number, u32 &imm)
{
  //Rs1 number stored as Bit 19-15
  rs1_number = ((word >> 15) & 0x1F);
  //Rs2 number stored as Bit 24-20
  rs2_number = ((word >> 20) & 0x1F);
  //Imm
  //Bit 11-5 of imm stored as Bit 31-25 in word --> right shift by 20 and remove everything else
  //Bit 4-0 of imm stored as Bit 11-7 in word --> right shift by 7 and remove everything else
  //Sign extend Bit 11 of imm to the left
  imm = ((word >> 20) & 0xFE0) | ((word >> 7) & 0x1F);

  imm = extendBit(imm, 11);
}

void CPU::getBTypeArgs(u32 word, u32 &rs1_number, u32 &rs2_number, u32 &imm)
{
  //Rs1 number stored as Bit 19-15
  rs1_number = ((word >> 15) & 0x1F);
  //Rs2 number stored as Bit 24-20
  rs2_number = ((word >> 20) & 0x1F);
  //Imm
  //Bit 12 of imm stored as Bit 31 in word --> right shift by 19 and remove everything else
  //Bit 11 of imm stored as Bit 7 in word --> left shift by 4 and remove everything else
  //Bit 10-5 of imm stored as Bit 30-25 in word --> right shift by 20 and remove everything else
  //Bit 4-1 of imm stored Bit 11-8 in word --> right shift by 7 and remove everything else
  //Bit 0 of imm = 0
  //Sign extend Bit 12 of imm to the left
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
#ifdef DEBUG
    printf("ADD: add 0x%x (reg %d) and 0x%x (reg %d)", registers_[rs1_number], rs1_number,
      registers_[rs2_number], rs2_number);
#endif
    registers_[rd_number] = registers_[rs1_number] + registers_[rs2_number];
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
  }

  return true;
}

bool CPU::ADDI(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
#ifdef DEBUG
    printf("ADDI: add 0x%x (reg %d) and 0x%x (imm)", registers_[rs1_number], rs1_number, imm);
#endif
    registers_[rd_number] = registers_[rs1_number] + imm;
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
  }

  return true;
}

bool CPU::AND(u32 word)
{
  u32 rd_number, rs1_number, rs2_number;
  getRTypeArgs(word, rd_number, rs1_number, rs2_number);

  if(rd_number != 0)
  {
#ifdef DEBUG
    printf("AND: and 0x%x (reg %d) and 0x%x (reg %d)", registers_[rs1_number], rs1_number,
      registers_[rs2_number], rs2_number);
#endif
    registers_[rd_number] = registers_[rs1_number] & registers_[rs2_number];
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
  }

  return true;
}

bool CPU::ANDI(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
#ifdef DEBUG
    printf("ANDI: and 0x%x (reg %d) and 0x%x (imm)", registers_[rs1_number], rs1_number, imm);
#endif
    registers_[rd_number] = registers_[rs1_number] & imm;
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
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
#ifdef DEBUG
    printf("AUPIC: add pc 0x%x and 0x%x (imm) to get 0x%x (reg %d)\n", pc_, imm, registers_[rd_number], rd_number);
#endif
  }

  return true;
}

bool CPU::BEQ(u32 word)
{
  u32 rs1_number, rs2_number, imm;
  getBTypeArgs(word, rs1_number, rs2_number, imm);

#ifdef DEBUG
  printf("BEQ:");
#endif

  if(registers_[rs1_number] == registers_[rs2_number])
  {
    pc_ += imm;
#ifdef DEBUG
    printf(" 0x%x (reg %d) equal to 0x%x (reg %d) so add pc 0x%x and 0x%x (imm) to get new pc 0x%x\n", registers_[rs1_number], rs1_number,
      registers_[rs2_number], rs2_number, pc_ - imm, imm, pc_);
#endif
    return false;
  }

#ifdef DEBUG
  printf(" do nothing because 0x%x (reg %d) != 0x%x (reg %d)\n", registers_[rs1_number], rs1_number,
    registers_[rs2_number], rs2_number);
#endif

  return true;
}

bool CPU::BGE(u32 word)
{
  u32 rs1_number, rs2_number, imm;
  getBTypeArgs(word, rs1_number, rs2_number, imm);

#ifdef DEBUG
  printf("BGE:");
#endif

  if((s32) registers_[rs1_number] >= (s32) registers_[rs2_number])
  {
    pc_ += imm;
#ifdef DEBUG
    printf(" 0x%x (reg %d) greater or equal to 0x%x (reg %d) so add pc 0x%x and 0x%x (imm) to get new pc 0x%x\n", registers_[rs1_number], rs1_number,
      registers_[rs2_number], rs2_number, pc_ - imm, imm, pc_);
#endif
    return false;
  }

#ifdef DEBUG
  printf(" do nothing because 0x%x (reg %d) < 0x%x (reg %d)\n", registers_[rs1_number], rs1_number,
    registers_[rs2_number], rs2_number);
#endif

  return true;
}

bool CPU::BGEU(u32 word)
{
  u32 rs1_number, rs2_number, imm;
  getBTypeArgs(word, rs1_number, rs2_number, imm);

#ifdef DEBUG
  printf("BGEU:");
#endif

  if(registers_[rs1_number] >= registers_[rs2_number])
  {
    pc_ += imm;
#ifdef DEBUG
    printf(" 0x%x (reg %d) greater or equal (unsigned) to 0x%x (reg %d) so add pc 0x%x and 0x%x (imm) to get new pc 0x%x\n", registers_[rs1_number], rs1_number,
      registers_[rs2_number], rs2_number, pc_ - imm, imm, pc_);
#endif
    return false;
  }

#ifdef DEBUG
  printf(" do nothing because 0x%x (reg %d) < 0x%x (reg %d) (unsigned)\n", registers_[rs1_number], rs1_number,
    registers_[rs2_number], rs2_number);
#endif

  return true;
}

bool CPU::BLT(u32 word)
{
  u32 rs1_number, rs2_number, imm;
  getBTypeArgs(word, rs1_number, rs2_number, imm);

#ifdef DEBUG
  printf("BLT:");
#endif

  if((s32) registers_[rs1_number] < (s32) registers_[rs2_number])
  {
    pc_ += imm;
#ifdef DEBUG
    printf(" 0x%x (reg %d) less than 0x%x (reg %d) so add pc 0x%x and 0x%x (imm) to get new pc 0x%x\n", registers_[rs1_number], rs1_number,
      registers_[rs2_number], rs2_number, pc_ - imm, imm, pc_);
#endif
    return false;
  }

#ifdef DEBUG
  printf(" do nothing because 0x%x (reg %d) >= 0x%x (reg %d)\n", registers_[rs1_number], rs1_number,
    registers_[rs2_number], rs2_number);
#endif

  return true;
}

bool CPU::BLTU(u32 word)
{
  u32 rs1_number, rs2_number, imm;
  getBTypeArgs(word, rs1_number, rs2_number, imm);

#ifdef DEBUG
  printf("BLTU:");
#endif

  if(registers_[rs1_number] < registers_[rs2_number])
  {
    pc_ += imm;
#ifdef DEBUG
    printf(" 0x%x (reg %d) less than (unsigned) 0x%x (reg %d) so add pc 0x%x and 0x%x (imm) to get new pc 0x%x\n", registers_[rs1_number], rs1_number,
      registers_[rs2_number], rs2_number, pc_ - imm, imm, pc_);
#endif
    return false;
  }

#ifdef DEBUG
  printf(" do nothing because 0x%x (reg %d) >= 0x%x (reg %d) (unsigned)\n", registers_[rs1_number], rs1_number,
    registers_[rs2_number], rs2_number);
#endif

  return true;
}

bool CPU::BNE(u32 word)
{
  u32 rs1_number, rs2_number, imm;
  getBTypeArgs(word, rs1_number, rs2_number, imm);

#ifdef DEBUG
  printf("BNE:");
#endif

  if(registers_[rs1_number] != registers_[rs2_number])
  {
    pc_ += imm;
#ifdef DEBUG
    printf(" 0x%x (reg %d) not equal to 0x%x (reg %d) so add pc 0x%x and 0x%x (imm) to get new pc 0x%x\n", registers_[rs1_number], rs1_number,
      registers_[rs2_number], rs2_number, pc_ - imm, imm, pc_);
#endif
    return false;
  }

#ifdef DEBUG
  printf(" do nothing because 0x%x (reg %d) == 0x%x (reg %d)\n", registers_[rs1_number], rs1_number,
    registers_[rs2_number], rs2_number);
#endif

  return true;
}

bool CPU::EBREAK(u32 word)
{
#ifdef DEBUG
  printf("EBREAK");
#endif
  exit(0);
}

bool CPU::JAL(u32 word)
{
  u32 rd_number, imm;
  getJTypeArgs(word, rd_number, imm);

  if(rd_number != 0)
  {
    registers_[rd_number] = pc_ + 4;
#ifdef DEBUG
    printf("JAL: store 0x%x (reg %d), ", registers_[rd_number], rd_number);
#endif
  }

#ifdef DEBUG
  printf("JAL: add pc 0x%x and 0x%x (imm) to get new pc 0x%x\n", pc_, imm, pc_ + imm);
#endif
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
#ifdef DEBUG
    printf("JALR: store 0x%x (reg %d), ", registers_[rd_number], rd_number);
#endif
  }

  pc_ = (registers_[rs1_number] + imm) & (~1);
#ifdef DEBUG
  printf("JALR: add 0x%x (reg %d) to 0x%x (imm) to get new pc 0x%x\n", registers_[rs1_number], rs1_number, imm, pc_);
#endif
  return false;
}

bool CPU::LB(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
    u32 address = (registers_[rs1_number] + imm) & 0xFF;
#ifdef DEBUG
    printf("LB: load from address 0x%x (reg %d) + 0x%x (imm) = 0x%x", registers_[rs1_number], rs1_number, imm, address);
#endif
    registers_[rd_number] = extendBit(read(address), 7);
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
  }

  return true;
}

bool CPU::LBU(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
    u32 address = (registers_[rs1_number] + imm) & 0xFF;
#ifdef DEBUG
    printf("LBU: load (unsigned) from address 0x%x (reg %d) + 0x%x (imm) = 0x%x", registers_[rs1_number], rs1_number, imm, address);
#endif
    registers_[rd_number] = read(address);
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
  }

  return true;
}

bool CPU::LH(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
    u32 address = registers_[rs1_number] + imm;
#ifdef DEBUG
    printf("LH: load from address 0x%x (reg %d) + 0x%x (imm) = 0x%x - 0x%x", registers_[rs1_number], rs1_number, imm,
      address, address + 1);
#endif
    u32 read_halfword = read(address) | ((((u32) read(address + 1)) << 8));
    registers_[rd_number] = extendBit(read_halfword, 15);
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
  }

  return true;
}

bool CPU::LHU(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
    u32 address = registers_[rs1_number] + imm;
#ifdef DEBUG
    printf("LHU: load (unsigned) from address 0x%x (reg %d) + 0x%x (imm) = 0x%x - 0x%x", registers_[rs1_number], rs1_number, imm,
      address, address + 1);
#endif
    u32 read_halfword = read(address) | ((((u32) read(address + 1)) << 8));
    registers_[rd_number] = read_halfword;
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
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
#ifdef DEBUG
    printf("LUI: set reg %d to 0x%x (imm)\n", rd_number, imm);
#endif
  }

  return true;
}

bool CPU::LW(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
    u32 address = registers_[rs1_number] + imm;
#ifdef DEBUG
    printf("LW: load from address 0x%x (reg %d) + 0x%x (imm) = 0x%x - 0x%x", registers_[rs1_number], rs1_number, imm,
      address, address + 3);
#endif
    u32 read_word = read(address) | (((u32) read(address + 1)) << 8) |
      (((u32) read(address + 2)) << 16) | (((u32) read(address + 3)) << 24);
    registers_[rd_number] = read_word;
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
  }

  return true;
}

bool CPU::OR(u32 word)
{
  u32 rd_number, rs1_number, rs2_number;
  getRTypeArgs(word, rd_number, rs1_number, rs2_number);

  if(rd_number != 0)
  {
#ifdef DEBUG
    printf("OR: or 0x%x (reg %d) and 0x%x (reg %d)", registers_[rs1_number], rs1_number,
      registers_[rs2_number], rs2_number);
#endif
    registers_[rd_number] = registers_[rs1_number] | registers_[rs2_number];
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
  }

  return true;
}

bool CPU::ORI(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
#ifdef DEBUG
    printf("ORI: or 0x%x (reg %d) and 0x%x (imm)", registers_[rs1_number], rs1_number, imm);
#endif
    registers_[rd_number] = registers_[rs1_number] | imm;
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
  }

  return true;
}

bool CPU::SB(u32 word)
{
  u32 rs1_number, rs2_number, imm;
  getSTypeArgs(word, rs1_number, rs2_number, imm);

  u32 address = registers_[rs1_number] + imm;
  write(address, (u8) registers_[rs2_number]);
#ifdef DEBUG
  printf("SB: write 0x%x (reg %d) to address 0x%x (reg %d) + 0x%x (imm) = 0x%x\n", (u8) registers_[rs2_number], rs2_number,
    registers_[rs1_number], rs1_number, imm, address);
#endif
  return true;
}

bool CPU::SH(u32 word)
{
  u32 rs1_number, rs2_number, imm;
  getSTypeArgs(word, rs1_number, rs2_number, imm);

  u32 address = registers_[rs1_number] + imm;
  write(address, (u8) registers_[rs2_number]);
  write(address + 1, (u8) (registers_[rs2_number] >> 8));
#ifdef DEBUG
  printf("SH: write 0x%x (reg %d) to address 0x%x (reg %d) + 0x%x (imm) = 0x%x - 0x%x\n", (u16) registers_[rs2_number],
    rs2_number, registers_[rs1_number], rs1_number, imm, address, address + 1);
#endif
  return true;
}

bool CPU::SLL(u32 word)
{
  u32 rd_number, rs1_number, rs2_number;
  getRTypeArgs(word, rd_number, rs1_number, rs2_number);

  if(rd_number != 0)
  {
#ifdef DEBUG
    printf("SLL: shift 0x%x (reg %d) left (logic) by 0x%x (reg %d)", registers_[rs1_number], rs1_number,
      registers_[rs2_number], rs2_number);
#endif
    registers_[rd_number] = registers_[rs1_number] << (registers_[rs2_number] % 32);
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
  }

  return true;
}

bool CPU::SLLI(u32 word)
{
  u32 rd_number, rs1_number, shamt;
  getI2TypeArgs(word, rd_number, rs1_number, shamt);

  if(rd_number != 0)
  {
#ifdef DEBUG
    printf("SLLI: shift 0x%x (reg %d) left (logic) by 0x%x (shamt)", registers_[rs1_number], rs1_number, shamt);
#endif
    registers_[rd_number] = registers_[rs1_number] << shamt;
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
  }

  return true;
}

bool CPU::SLT(u32 word)
{
  u32 rd_number, rs1_number, rs2_number;
  getRTypeArgs(word, rd_number, rs1_number, rs2_number);

  if(rd_number != 0)
  {
#ifdef DEBUG
    printf("SLT: 0x%x (reg %d) <?> 0x%x (reg %d)", registers_[rs1_number], rs1_number,
      registers_[rs2_number], rs2_number);
#endif
    registers_[rd_number] = ((s32) registers_[rs1_number] < (s32) registers_[rs2_number]) ? 1 : 0;
#ifdef DEBUG
    printf(" so set reg %d to 0x%x\n", rd_number, registers_[rd_number]);
#endif
  }

  return true;
}

bool CPU::SLTI(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
#ifdef DEBUG
    printf("SLTI: 0x%x (reg %d) <?> 0x%x (imm)", registers_[rs1_number], rs1_number, imm);
#endif
    registers_[rd_number] = ((s32) registers_[rs1_number] < (s32) imm) ? 1 : 0;
#ifdef DEBUG
    printf(" so set reg %d to 0x%x\n", rd_number, registers_[rd_number]);
#endif
  }

  return true;
}

bool CPU::SLTIU(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
#ifdef DEBUG
    printf("SLTIU: 0x%x (reg %d) <?> 0x%x (imm) (unsigned)", registers_[rs1_number], rs1_number, imm);
#endif
    registers_[rd_number] = (registers_[rs1_number] < imm) ? 1 : 0;
#ifdef DEBUG
    printf(" so set reg %d to 0x%x\n", rd_number, registers_[rd_number]);
#endif
  }

  return true;
}

bool CPU::SLTU(u32 word)
{
  u32 rd_number, rs1_number, rs2_number;
  getRTypeArgs(word, rd_number, rs1_number, rs2_number);

  if(rd_number != 0)
  {
#ifdef DEBUG
    printf("SLT: 0x%x (reg %d) <?> 0x%x (reg %d)", registers_[rs1_number], rs1_number,
      registers_[rs2_number], rs2_number);
#endif
    registers_[rd_number] = (registers_[rs1_number] < registers_[rs2_number]) ? 1 : 0;
#ifdef DEBUG
    printf(" so set reg %d to 0x%x\n", rd_number, registers_[rd_number]);
#endif
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
#ifdef DEBUG
    printf("SRA: shift 0x%x (reg %d) right (arithmetic) by 0x%x (reg %d)", registers_[rs1_number], rs1_number,
      registers_[rs2_number], rs2_number);
#endif
    registers_[rd_number] = extendBit(registers_[rs1_number] >> shift, (31 - shift));
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
  }

  return true;
}

bool CPU::SRAI(u32 word)
{
  u32 rd_number, rs1_number, shamt;
  getI2TypeArgs(word, rd_number, rs1_number, shamt);

  if(rd_number != 0)
  {
#ifdef DEBUG
    printf("SRAI: shift 0x%x (reg %d) right (arithmetic) by 0x%x (shamt)", registers_[rs1_number], rs1_number, shamt);
#endif
    registers_[rd_number] = extendBit(registers_[rs1_number] >> shamt, (31 - shamt));
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
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
#ifdef DEBUG
    printf("SRL: shift 0x%x (reg %d) right (logic) by 0x%x (reg %d) logically", registers_[rs1_number], rs1_number,
      registers_[rs2_number], rs2_number);
#endif
    registers_[rd_number] = registers_[rs1_number] >> shift;
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
  }

  return true;
}

bool CPU::SRLI(u32 word)
{
  u32 rd_number, rs1_number, shamt;
  getI2TypeArgs(word, rd_number, rs1_number, shamt);

  if(rd_number != 0)
  {
#ifdef DEBUG
    printf("SRLI: shift 0x%x (reg %d) right (logic) by 0x%x (shamt) logically", registers_[rs1_number], rs1_number, shamt);
#endif
    registers_[rd_number] = registers_[rs1_number] >> shamt;
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
  }

  return true;
}

bool CPU::SUB(u32 word)
{
  u32 rd_number, rs1_number, rs2_number;
  getRTypeArgs(word, rd_number, rs1_number, rs2_number);

  if(rd_number != 0)
  {
#ifdef DEBUG
    printf("SUB: subtract 0x%x (reg %d) from 0x%x (reg %d)", registers_[rs2_number], rs2_number,
      registers_[rs1_number], rs1_number);
#endif
    registers_[rd_number] = registers_[rs1_number] - registers_[rs2_number];
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
  }

  return true;
}

bool CPU::SW(u32 word)
{
  u32 rs1_number, rs2_number, imm;
  getSTypeArgs(word, rs1_number, rs2_number, imm);

  u32 address = registers_[rs1_number] + imm;
  write(address, (u8) registers_[rs2_number]);
  write(address + 1, (u8) (registers_[rs2_number] >> 8));
  write(address + 2, (u8) (registers_[rs2_number] >> 16));
  write(address + 3, (u8) (registers_[rs2_number] >> 24));
#ifdef DEBUG
  printf("SW: write 0x%x (reg %d) to address 0x%x (reg %d) + 0x%x (imm) = 0x%x - 0x%x\n", registers_[rs2_number],
    rs2_number, registers_[rs1_number], rs1_number, imm, address, address + 3);
#endif
  return true;
}

bool CPU::XOR(u32 word)
{
  u32 rd_number, rs1_number, rs2_number;
  getRTypeArgs(word, rd_number, rs1_number, rs2_number);

  if(rd_number != 0)
  {
#ifdef DEBUG
    printf("XOR: xor 0x%x (reg %d) and 0x%x (reg %d)", registers_[rs1_number], rs1_number,
      registers_[rs2_number], rs2_number);
#endif
    registers_[rd_number] = registers_[rs1_number] ^ registers_[rs2_number];
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
  }

  return true;
}

bool CPU::XORI(u32 word)
{
  u32 rd_number, rs1_number, imm;
  getITypeArgs(word, rd_number, rs1_number, imm);

  if(rd_number != 0)
  {
#ifdef DEBUG
    printf("XORI: xor 0x%x (reg %d) and 0x%x (imm)", registers_[rs1_number], rs1_number, imm);
#endif
    registers_[rd_number] = registers_[rs1_number] ^ imm;
#ifdef DEBUG
    printf(" to get 0x%x (reg %d)\n", registers_[rd_number], rd_number);
#endif
  }

  return true;
}

u32 CPU::getPC()
{
  return pc_;
}

u32 CPU::getSP()
{
  return registers_[2];
}

std::vector<u8> CPU::getMemory()
{
  return memory_;
}

