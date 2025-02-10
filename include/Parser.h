#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <string>
#include "definitions.h"

class Parser
{
  public:
    Parser();
    virtual ~Parser();
    void parse(std::vector<Instruction> &instructions, std::string &filename);
    std::map<std::string, Label> label_lut_;

  private:
    Instruction getInstruction(std::vector<std::string> &split_line);
    std::vector<std::string> tokenizeLine(std::string &line);
    std::map<std::string, int> register_lut_;
    std::map<std::string, InstructionInfo> info_lut_;
    u32 line_count_ = 0;

    void reset();
    void parseLabel(std::vector<std::string> &split_line);
    void parseUType(Instruction &instruction, std::vector<std::string> &split_line);
    void parseJType(Instruction &instruction, std::vector<std::string> &split_line);
    void parseRType(Instruction &instruction, std::vector<std::string> &split_line);
    void parseIType(Instruction &instruction, std::vector<std::string> &split_line);
    void parseSType(Instruction &instruction, std::vector<std::string> &split_line);
    void parseBType(Instruction &instruction, std::vector<std::string> &split_line);
    void getImm(int &imm, std::string &line);

    void toLower(std::string &line);
    void toUpper(std::string &line);

    bool isHex(std::string &line);
};
