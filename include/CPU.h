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


  private:
    u32 registers_[32];
    u32 pc_;
    std::vector<u8> memory_;

    void ADD(u32 word);
    void ADDI(u32 word);
    void AND(u32 word);
    void ANDI(u32 word);
    void AUIPC(u32 word);
    void BEQ(u32 word);
    void BGE(u32 word);
    void BGEU(u32 word);
    void BLT(u32 word);
    void BLTU(u32 word);
    void BNE(u32 word);
    void EBREAK(u32 word);
    void JAL(u32 word);
    void JALR(u32 word);
    void LB(u32 word);
    void LBU(u32 word);
    void LH(u32 word);
    void LHU(u32 word);
    void LUI(u32 word);
    void LW(u32 word);
    void OR(u32 word);
    void ORI(u32 word);
    void SB(u32 word);
    void SH(u32 word);
    void SLL(u32 word);
    void SLLI(u32 word);
    void SLT(u32 word);
    void SLTI(u32 word);
    void SLTIU(u32 word);
    void SLTU(u32 word);
    void SRA(u32 word);
    void SRAI(u32 word);
    void SRL(u32 word);
    void SRLI(u32 word);
    void SUB(u32 word);
    void SW(u32 word);
    void XOR(u32 word);
    void XORI(u32 word);

    typedef void (CPU::*function_ptr)(u32);
    std::map<InstructionInfo, function_ptr> instruction_lut_;
};
