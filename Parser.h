#include <vector>
#include <map>
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
    std::vector<std::string> splitLine(std::string &line);
    std::fstream file_;
    std::map<std::string, int> lut_;
};
