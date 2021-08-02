#include <string>
#include "../include/CPU.h"

class Assembler
{
  public:
    Assembler(CPU &cpu);
    virtual ~Assembler();
    void assemble(std::string file_name);

  private:
    CPU &cpu_;
    Parser parser_;
    void assignLabelAddress(std::vector<Instruction> &instructions);
    u32 getHexRepresentation(Instruction &instruction);
    void UType(u32 &hex, Instruction &instruction);
    void JType(u32 &hex, Instruction &instruction);
    void RType(u32 &hex, Instruction &instruction);
    void IType(u32 &hex, Instruction &instruction);
    void I2Type(u32 &hex, Instruction &instruction);
    void SType(u32 &hex, Instruction &instruction);
    void BType(u32 &hex, Instruction &instruction);
};