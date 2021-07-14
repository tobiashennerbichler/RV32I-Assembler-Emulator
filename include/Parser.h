#include <vector>
#include <map>
#include <string>
#include <fstream>
#include "definitions.h"

class Parser
{
  public:
    Parser();
    virtual ~Parser();
    bool parse(std::vector<Instruction> &instructions, std::string &filename);
    std::map<std::string, Label> label_lut_;

  private:
    Instruction getInstruction(std::string &line);
    std::vector<std::string> splitLine(std::string &line);
    std::map<std::string, int> lut_;
    int label_count_ = 0;

    void parseJAL(Instruction &instruction, std::vector<std::string> &split_line);
    void parseADD(Instruction &instruction, std::vector<std::string> &split_line);
    void parseLW(Instruction &instruction, std::vector<std::string> &split_line);
    void parseSW(Instruction &instruction, std::vector<std::string> &split_line);
    void parseADDI(Instruction &instruction, std::vector<std::string> &split_line);
    void parseBEQ(Instruction &instruction, std::vector<std::string> &split_line);

    bool isHex(std::string &line);
};
