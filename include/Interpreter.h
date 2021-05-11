#include <vector>
#include "Parser.h"

class Interpreter
{
  public:
    Interpreter();
    void run();

  private:
    void ADD(u32 &Rd, u32 Rs1, u32 Rs2);
    void ADDI(u32 &Rd, u32 Rs1, u32 imm);
    void SUB(u32 &Rd, u32 Rs1, u32 Rs2);
    void LW(u32 &Rd, u32 Rs1, u32 imm);
    void SW(u32 Rs1, u32 Rs2, u32 imm);
    void JAL(u32 &Rd, u32 imm);
    void JALR(u32 Rd, u32 Rs1, u32 imm);
    void BEQ(u32 Rs1, u32 Rs2, u32 imm);
    void BNE(u32 Rs1, u32 Rs2, u32 imm);
    void BLT(u32 Rs1, u32 Rs2, u32 imm);
    void BGE(u32 Rs1, u32 Rs2, u32 imm);
    [[noreturn]] void EBREAK();

    void executeInstruction(Instruction &instruction);

    std::vector<u32> memory_;
    u32 pc_;
    std::vector<u32> registers_;
    std::vector<Instruction> instructions_;
};