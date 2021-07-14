#include <string>
#include "../include/CPU.h"

class Assembler
{
  public:
    Assembler(CPU &cpu, std::string &filename);
    virtual ~Assembler();

  private:
    CPU &cpu_;
    Parser parser_;
    std::string file_name_;
    std::map<std::string, int> opcodes_;
    void assemble();
    void assignLabelAddress(std::vector<Instruction> &instructions);
    u32 getHexRepresentation(Instruction &instruction);
};