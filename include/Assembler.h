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
    std::map<std::string, InstrInfo> opcodes_;
    void assignLabelAddress(std::vector<Instruction> &instructions);
    u32 getHexRepresentation(Instruction &instruction);
};