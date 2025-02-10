#include "../include/Parser.h"
#include <iostream>
#include <fstream>

Parser::Parser()
{
  //use this look-up table to translate string representation of registers into its index
  register_lut_ = {
    {"zero", 0}, {"ra", 1}, {"sp", 2}, {"gp", 3}, {"tp", 4}, {"t0", 5},
    {"t1", 6}, {"t2", 7}, {"fp", 8}, {"s0", 8}, {"s1", 9}, {"a0", 10}, {"a1", 11},
    {"a2", 12}, {"a3", 13}, {"a4", 14}, {"a5", 15}, {"a6", 16}, {"a7", 17},
    {"s2", 18}, {"s3", 19}, {"s4", 20}, {"s5", 21}, {"s6", 22}, {"s7", 23},
    {"s8", 24}, {"s9", 25}, {"s10", 26}, {"s11", 27}, {"t3", 28}, {"t4", 29},
    {"t5", 30}, {"t6", 31}
  };

  //use this look-up table to get the corresponding instruction info used for assembling for the given instruction
  info_lut_ = {
    {"EBREAK", {0x73, 0x00, 0x00, I_TYPE}},
    {"LUI", {0x37, 0x00, 0x00, U_TYPE}},
    {"AUIPC", {0x17, 0x00, 0x00, U_TYPE}},
    {"JAL", {0x6F, 0x00, 0x00, J_TYPE}},
    {"JALR", {0x67, 0x00, 0x00, I_TYPE}},
    {"BEQ", {0x63, 0x00, 0x00, B_TYPE}},
    {"BNE", {0x63, 0x01, 0x00, B_TYPE}},
    {"BLT", {0x63, 0x04, 0x00, B_TYPE}},
    {"BGE", {0x63, 0x05, 0x00, B_TYPE}},
    {"BLTU", {0x63, 0x06, 0x00, B_TYPE}},
    {"BGEU", {0x63, 0x07, 0x00, B_TYPE}},
    {"LB", {0x03, 0x00, 0x00, I_TYPE}},
    {"LH", {0x03, 0x01, 0x00, I_TYPE}},
    {"LW", {0x03, 0x02, 0x00, I_TYPE}},
    {"LBU", {0x03, 0x04, 0x00, I_TYPE}},
    {"LHU", {0x03, 0x05, 0x00, I_TYPE}},
    {"SB", {0x23, 0x00, 0x00, S_TYPE}},
    {"SH", {0x23, 0x01, 0x00, S_TYPE}},
    {"SW", {0x23, 0x02, 0x00, S_TYPE}},
    {"ADDI", {0x13, 0x00, 0x00, I_TYPE}},
    {"SLTI", {0x13, 0x02, 0x00, I_TYPE}},
    {"SLTIU", {0x13, 0x03, 0x00, I_TYPE}},
    {"XORI", {0x13, 0x04, 0x00, I_TYPE}},
    {"ORI", {0x13, 0x06, 0x00, I_TYPE}},
    {"ANDI", {0x13, 0x07, 0x00, I_TYPE}},
    {"SLLI", {0x13, 0x01, 0x00, I_TYPE2}},
    {"SRLI", {0x13, 0x05, 0x00, I_TYPE2}},
    {"SRAI", {0x13, 0x05, 0x20, I_TYPE2}},
    {"ADD", {0x33, 0x00, 0x00, R_TYPE}},
    {"SUB", {0x33, 0x00, 0x20, R_TYPE}},
    {"SLL", {0x33, 0x01, 0x00, R_TYPE}},
    {"SLT", {0x33, 0x02, 0x00, R_TYPE}},
    {"SLTU", {0x33, 0x03, 0x00, R_TYPE}},
    {"XOR", {0x33, 0x04, 0x00, R_TYPE}},
    {"SRL", {0x33, 0x05, 0x00, R_TYPE}},
    {"SRA", {0x33, 0x05, 0x20, R_TYPE}},
    {"OR", {0x33, 0x06, 0x00, R_TYPE}},
    {"AND", {0x33, 0x07, 0x00, R_TYPE}}
  };
}

Parser::~Parser()
{

}

void Parser::reset()
{
  label_lut_.clear();
  line_count_ = 0;
}

/*
 * opens the given file and separates the labels from the instructions
 * labels are stored in a separate look-up table with the address of the next instruction
 * instructions are parsed depending on their type
 * if not all labels are verified in the end, the program is not valid
 */
void Parser::parse(std::vector<Instruction> &instructions, std::string &filename)
{
  reset();
  std::fstream file = std::fstream(filename, std::ios::in);

  if(!file.is_open())
  {
    std::cout << "File " << filename << " could not be opened" << std::endl;
    exit(-1);
  }

  std::string line;
  while(std::getline(file, line))
  {
    std::vector<std::string> tokens = tokenizeLine(line);
    if(tokens.size() == 0) {
      continue;
    }

    //interpret single statements with an ':' at the end as a label
    if(tokens.at(0).back() == ':')
    {
      parseLabel(tokens);
      continue;
    }

    Instruction instruction = getInstruction(tokens);
    #ifdef DEBUG
    printf("line %d: %s - Rd: %d - Rs1: %d - Rs2: %d - imm: %d - label: %s\n", line_count_ + 1, instruction.name_.c_str(),
      instruction.Rd_, instruction.Rs1_, instruction.Rs2_, instruction.imm_, instruction.label_name_.c_str());
    #endif
    line_count_++;
    instructions.push_back(instruction);
  }

  for(auto &label : label_lut_)
  {
    if(!label.second.verified_)
    {
      std::cout << "Label " << label.first << " " << label.first << " not verified" << std::endl;
      exit(-1);
    }
  }
}

/*
 * Finds the instruction info for the current instruction in the look-up table if it exists
 * Depending on the instruction info, it checks for correctness of the used instruction and parses
 * the corresponding arguments
 */
//TODO: make error messages better, write line in which error occurred and which error occurred
Instruction Parser::getInstruction(std::vector<std::string> &tokens)
{
  if(tokens.empty())
  {
    std::cout << "Invalid line (empty)" << std::endl;
    exit(-1);
  }

  toUpper(tokens.at(0));

  auto instruction_info = info_lut_.find(tokens.at(0));
  if(instruction_info == info_lut_.end())
  {
    std::cout << "Unknown instruction: " << tokens.at(0) << std::endl;
    exit(-1);
  }

  Instruction instruction = {tokens.at(0), 0, 0, 0, 0, "",
    instruction_info->second};

  switch(instruction.info_.type_)
  {
    case U_TYPE:
      if(tokens.size() != 3)
      {
        std::cout << "UType: Invalid line: " << tokens.size() << std::endl;
        exit(-1);
      }

      parseUType(instruction, tokens);
      break;
    case J_TYPE:
      if(tokens.size() != 3)
      {
        std::cout << "JType: Invalid line: " << tokens.size() << std::endl;
        exit(-1);
      }

      parseJType(instruction, tokens);
      break;
    case R_TYPE:
      if(tokens.size() != 4)
      {
        std::cout << "RType: Invalid line: " << tokens.size() << std::endl;
        exit(-1);
      }

      parseRType(instruction, tokens);
      break;
    case I_TYPE:
    case I_TYPE2:
      if(tokens.size() == 1 && instruction.name_ == "EBREAK")
      {
        instruction.imm_ = 0x1;
        break;
      }

      if(tokens.size() != 4)
      {
        std::cout << "IType/IType2: Invalid line: " << tokens.size() << std::endl;
        exit(-1);
      }

      parseIType(instruction, tokens);
      break;
    case S_TYPE:
      if(tokens.size() != 4)
      {
        std::cout << "SType: Invalid line: " << tokens.size() << std::endl;
        exit(-1);
      }

      parseSType(instruction, tokens);
      break;
    case B_TYPE:
      if(tokens.size() != 4)
      {
        std::cout << "BType: Invalid line: " << tokens.size() << std::endl;
        exit(-1);
      }

      parseBType(instruction, tokens);
      break;
  }

  return instruction;
}

/*
 * splits the line with delimiters (' ', ',', '(', ')' and '\t')
 */
//TODO: interpret '#' as comments
std::vector<std::string> Parser::tokenizeLine(std::string &line)
{
  std::vector<std::string> tokens;
  std::regex delimiters("[ ,()\t\r\n]");
  auto it_begin = std::sregex_token_iterator(line.begin(), line.end(), delimiters, -1);
  auto it_end = std::sregex_token_iterator();
  for(auto it = it_begin; it != it_end; it++) {
    std::string match = it->str();
    if(match.empty()) {
      continue;
    }

    tokens.push_back(match);
  }

  return tokens;
}

/*
 * Parses the label by storing its name without the ':' and storing it in the look-up table if it does not
 * already exist and verify it
 * If it already exists, verify the label
 */
void Parser::parseLabel(std::vector<std::string> &tokens)
{
  if(tokens.at(0).size() <= 1 || tokens.size() != 1)
  {
    std::cout << "Not a label" << std::endl;
    exit(-1);
  }

  //store label without ':'
  std::string label_name = tokens.at(0).substr(0, tokens.at(0).size() - 1);

  auto label = label_lut_.find(label_name);

  if(label == label_lut_.end())
  {
    label_lut_.insert({label_name, {true, line_count_ * 4}});
  }
  else
  {
    label->second.verified_ = true;
    label->second.address_ = line_count_ * 4;
  }
}

/*
 * The following functions parse the strings depending on the instruction type
 * If a J(ump) or B(ranch) type is parsed, then add the label to the look-up table but do not verify it yet
 * as the label has to appear as a single statement for the program to be correct
 */
void Parser::parseUType(Instruction &instruction, std::vector<std::string> &tokens)
{
  toLower(tokens.at(1));
  auto Rd = register_lut_.find(tokens.at(1));
  int imm = 0;

  if(Rd == register_lut_.end())
  {
    std::cout << "Error with register parsing in UType" << std::endl;
    exit(-1);
  }

  getImm(imm, tokens.at(2));

  instruction.Rd_ = Rd->second;
  instruction.imm_ = imm;
}

void Parser::parseJType(Instruction &instruction, std::vector<std::string> &tokens)
{
  toLower(tokens.at(1));
  auto Rd = register_lut_.find(tokens.at(1));
  auto label = label_lut_.find(tokens.at(2));

  if(Rd == register_lut_.end())
  {
    std::cout << "Error with register parsing in JType" << std::endl;
    exit(-1);
  }

  if(label == label_lut_.end())
  {
    label_lut_.insert({tokens.at(2), {false}});
  }

  instruction.Rd_ = Rd->second;
  instruction.label_name_ = tokens.at(2);
}

void Parser::parseRType(Instruction &instruction, std::vector<std::string> &tokens)
{
  toLower(tokens.at(1));
  toLower(tokens.at(2));
  toLower(tokens.at(3));
  auto Rd = register_lut_.find(tokens.at(1));
  auto Rs1 = register_lut_.find(tokens.at(2));
  auto Rs2 = register_lut_.find(tokens.at(3));

  if(Rd == register_lut_.end() || Rs1 == register_lut_.end() || Rs2 == register_lut_.end())
  {
    std::cout << "Error with register parsing in RType" << std::endl;
    exit(-1);
  }

  instruction.Rd_ = Rd->second;
  instruction.Rs1_ = Rs1->second;
  instruction.Rs2_ = Rs2->second;
}

void Parser::parseIType(Instruction &instruction, std::vector<std::string> &tokens)
{
  toLower(tokens.at(1));
  auto Rd = register_lut_.find(tokens.at(1));
  auto Rs1 = register_lut_.begin();
  int imm = 0;

  if(instruction.name_ == "ADDI" || instruction.name_ == "ANDI" || instruction.name_ == "SLLI" ||
    instruction.name_ == "SLTI" || instruction.name_ == "SLTIU" || instruction.name_ == "SRAI" ||
    instruction.name_ == "SRLI" || instruction.name_ == "XORI")
  {
    toLower(tokens.at(2));
    Rs1 = register_lut_.find(tokens.at(2));
    getImm(imm, tokens.at(3));
  }
  else
  {
    toLower(tokens.at(3));
    getImm(imm, tokens.at(2));
    Rs1 = register_lut_.find(tokens.at(3));
  }

  if(Rd == register_lut_.end() || Rs1 == register_lut_.end())
  {
    std::cout << "Error with register parsing in IType" << std::endl;
    exit(-1);
  }

  instruction.Rd_ = Rd->second;
  instruction.Rs1_ = Rs1->second;
  instruction.imm_ = imm;
}

void Parser::parseSType(Instruction &instruction, std::vector<std::string> &tokens)
{
  toLower(tokens.at(1));
  toLower(tokens.at(3));
  auto Rs1 = register_lut_.find(tokens.at(3));
  auto Rs2 = register_lut_.find(tokens.at(1));
  int imm = 0;

  getImm(imm, tokens.at(2));

  if(Rs1 == register_lut_.end() || Rs2 == register_lut_.end())
  {
    std::cout << "Rs1: " << tokens.at(3) << " Rs2: " << tokens.at(1) << std::endl;
    std::cout << "Error with register parsing in SType" << std::endl;
    exit(-1);
  }

  instruction.Rs1_ = Rs1->second;
  instruction.Rs2_ = Rs2->second;
  instruction.imm_ = imm;
}

void Parser::parseBType(Instruction &instruction, std::vector<std::string> &tokens)
{
  toLower(tokens.at(1));
  toLower(tokens.at(2));
  auto Rs1 = register_lut_.find(tokens.at(1));
  auto Rs2 = register_lut_.find(tokens.at(2));
  auto label = label_lut_.find(tokens.at(3));

  if(Rs1 == register_lut_.end() || Rs2 == register_lut_.end())
  {
    std::cout << "Error with register parsing in BType" << std::endl;
    exit(-1);
  }

  if(label == label_lut_.end())
  {
    label_lut_.insert({tokens.at(3), {false}});
  }

  instruction.Rs1_ = Rs1->second;
  instruction.Rs2_ = Rs2->second;
  instruction.label_name_ = tokens.at(3);
}

void Parser::toLower(std::string &line)
{
  std::transform(line.begin(), line.end(), line.begin(), [](char c) { return std::tolower(c); });
}

void Parser::toUpper(std::string &line)
{
  std::transform(line.begin(), line.end(), line.begin(), [](char c) { return std::toupper(c); });
}

/*
 * Used to check if the given string contains a hexadecimal number
 */
bool Parser::isHex(std::string &line)
{
  if(line.size() >= 2 && line.at(0) == '0' && line.at(1) == 'x')
  {
    if(line.size() == 2)
    {
      std::cout << "line " << line_count_ << ": imm " << line << " wrong" << std::endl;
      exit(-1);
    }

    return true;
  }

  return false;
}

/*
 * Try to get the integer representation of the given immediate string
 */
void Parser::getImm(int &imm, std::string &line)
{
  int base = isHex(line) ? 16 : 10;

  try {
    imm = std::stoi(line, nullptr, base);
  }
  catch(std::exception &e) {
    std::cout << "Error with immediate parsing: " << e.what() << std::endl;
    exit(-1);
  }
}
