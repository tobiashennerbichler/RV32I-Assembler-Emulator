#include <vector>
#include <string>
#include <fstream>
#include "definitions.h"

class Parser
{
  public:
    Parser();
    void parse(std::vector<Instruction> &instructions, std::string filename);

  private:
    Instruction getInstruction(std::string line);
    std::fstream file_;
};
