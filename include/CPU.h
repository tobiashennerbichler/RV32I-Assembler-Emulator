#include <vector>
#include <map>
#include "definitions.h"

class CPU
{
  public:
    CPU();
    virtual ~CPU();
    u8 read(u32 address);
    void write(u32 address, u8 data);
    void tick();
    u32 getPC();
    u32 getSP();
    std::vector<u8> getMemory();

  private:
    u32 registers_[32];
    u32 pc_ = 0;
    u32 memory_size_;
    std::vector<u8> memory_;

    InstructionInfo getInfo(u32 word);
    void getUTypeArgs(u32 word, u32 &rd_number, u32 &imm);
    void getJTypeArgs(u32 word, u32 &rd_number, u32 &imm);
    void getRTypeArgs(u32 word, u32 &rd_number, u32 &rs1_number, u32 &rs2_number);
    void getITypeArgs(u32 word, u32 &rd_number, u32 &rs1_number, u32 &imm);
    void getI2TypeArgs(u32 word, u32 &rd_number, u32 &rs1_number, u32 &shamt);
    void getSTypeArgs(u32 word, u32 &rs1_number, u32 &rs2_number, u32 &imm);
    void getBTypeArgs(u32 word, u32 &rs1_number, u32 &rs2_number, u32 &imm);

    u32 extendBit(u32 word, u8 bit);

    bool ADD(u32 word);
    bool ADDI(u32 word);
    bool AND(u32 word);
    bool ANDI(u32 word);
    bool AUIPC(u32 word);
    bool BEQ(u32 word);
    bool BGE(u32 word);
    bool BGEU(u32 word);
    bool BLT(u32 word);
    bool BLTU(u32 word);
    bool BNE(u32 word);
    bool EBREAK(u32 word);
    bool JAL(u32 word);
    bool JALR(u32 word);
    bool LB(u32 word);
    bool LBU(u32 word);
    bool LH(u32 word);
    bool LHU(u32 word);
    bool LUI(u32 word);
    bool LW(u32 word);
    bool OR(u32 word);
    bool ORI(u32 word);
    bool SB(u32 word);
    bool SH(u32 word);
    bool SLL(u32 word);
    bool SLLI(u32 word);
    bool SLT(u32 word);
    bool SLTI(u32 word);
    bool SLTIU(u32 word);
    bool SLTU(u32 word);
    bool SRA(u32 word);
    bool SRAI(u32 word);
    bool SRL(u32 word);
    bool SRLI(u32 word);
    bool SUB(u32 word);
    bool SW(u32 word);
    bool XOR(u32 word);
    bool XORI(u32 word);

    typedef bool (CPU::*function_ptr)(u32);
    std::map<InstructionInfo, function_ptr> instruction_lut_;
};
