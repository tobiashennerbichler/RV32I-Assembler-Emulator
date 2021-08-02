#include "../include/Parser.h"
#include <iostream>
#include <fstream>

Parser::Parser()
{
  //use this look-up table to translate string representation of registers into its index
  register_lut_ = {
    {"zero", 0}, {"ra", 1}, {"sp", 2}, {"gp", 3}, {"tp", 4}, {"t0", 5},
    {"t1", 6}, {"t2", 7}, {"fp", 8}, {"s1", 9}, {"a0", 10}, {"a1", 11},
    {"a2", 12}, {"a3", 13}, {"a4", 14}, {"a5", 15}, {"a6", 16}, {"a7", 17},
    {"s2", 18}, {"s3", 19}, {"s4", 20}, {"s5", 21}, {"s6", 22}, {"s7", 23},
    {"s8", 24}, {"s9", 25}, {"s10", 26}, {"s11", 27}, {"t3", 28}, {"t4", 29},
    {"t5", 30}, {"t6", 31}
  };

  info_lut_ = {
    {"EBREAK", {0x73, 0x00, 0x00, I_TYPE}},
    {"LUI", {0x37, 0x00, 0x00, U_TYPE}},
    {"AUIPC", {0x37, 0x00, 0x00, U_TYPE}},
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
 * goes through the given file and separates each statement into instructions
 * param instructions: vector that saves instructions for every line
 * param filename: filename for file to be parsed
 * return: true when file has been parsed
 * return: false when file could not be opened
 */
bool Parser::parse(std::vector<Instruction> &instructions, std::string &filename)
{
  reset();
  std::fstream file = std::fstream(filename, std::ios::in);

  if(!file.is_open())
  {
    printf("file %s could not be opened\n", filename.c_str());
    return false;
  }

  std::string line;
  while(std::getline(file, line))
  {
    if(!line.empty())
    {
      std::vector<std::string> split_line = splitLine(line);

      //interpret single statements with an ':' at the end as a label
      if(split_line.at(0).back() == ':')
      {
        parseLabel(split_line);
        continue;
      }

      Instruction instruction = getInstruction(split_line);
      printf("name: %s, Rd: %d, Rs1: %d, Rs2: %d, imm: %d, label: %s\n", instruction.name_.c_str(), instruction.Rd_, instruction.Rs1_,
        instruction.Rs2_, instruction.imm_, instruction.label_name_.c_str());
      instructions.push_back(instruction);
      line_count_++;
    }
  }

  for(auto &label : label_lut_)
  {
    if(!label.second.verified_)
    {
      printf("label %s not verified\n", label.first.c_str());
      exit(-1);
    }
  }

  return true;
}

/*
 * parses a line into instructions and checks for errors
 * param line: line to be parsed
 * return: Instructions in line
 */
Instruction Parser::getInstruction(std::vector<std::string> &split_line)
{
  if(split_line.empty())
  {
    printf("Invalid line (empty)\n");
    exit(-1);
  }

  auto instruction_info = info_lut_.find(split_line.at(0));

  if(instruction_info == info_lut_.end())
  {
    printf("Unknown instruction: %s\n", split_line.at(0).c_str());
    exit(-1);
  }

  Instruction instruction = {split_line.at(0), 0, 0, 0, 0, "",
    instruction_info->second};

  switch(instruction.info_.type_)
  {
    case U_TYPE:
      if(split_line.size() != 3)
      {
        printf("UType: Invalid line: %ld\n", split_line.size());
        exit(-1);
      }

      parseUType(instruction, split_line);
      break;
    case J_TYPE:
      if(split_line.size() != 3)
      {
        printf("JType: Invalid line: %ld\n", split_line.size());
        exit(-1);
      }

      parseJType(instruction, split_line);
      break;
    case R_TYPE:
      if(split_line.size() != 4)
      {
        printf("RType: Invalid line: %ld\n", split_line.size());
        exit(-1);
      }

      parseRType(instruction, split_line);
      break;
    case I_TYPE:
    case I_TYPE2:
      if(split_line.size() == 1 && instruction.name_ == "EBREAK")
      {
        instruction.imm_ = 0x1;
        break;
      }

      if(split_line.size() != 4)
      {
        printf("IType/IType2: Invalid line: %ld\n", split_line.size());
        exit(-1);
      }

      parseIType(instruction, split_line);
      break;
    case S_TYPE:
      if(split_line.size() != 4)
      {
        printf("SType: Invalid line: %ld\n", split_line.size());
        exit(-1);
      }

      parseSType(instruction, split_line);
      break;
    case B_TYPE:
      if(split_line.size() != 4)
      {
        printf("BType: Invalid line: %ld\n", split_line.size());
        exit(-1);
      }

      parseBType(instruction, split_line);
      break;
  }

  return instruction;
}

/*
 * splits the line with delimiters (' ', ',', '(' and ')')
 */
//TODO: interpret '#' as comments
std::vector<std::string> Parser::splitLine(std::string &line)
{
  std::vector<std::string> attributes;
  line.push_back(' ');
  std::string split;

  for(auto &s : line)
  {
    if(s == ' ' || s == ',' || s == '(' || s == ')')
    {
      if(!split.empty())
      {
        attributes.push_back(split);
      }
      split = "";
    }
    else
    {
      split.push_back(s);
    }
  }

  return attributes;
}

void Parser::parseLabel(std::vector<std::string> &split_line)
{
  if(split_line.at(0).size() <= 1)
  {
    printf("label too small");
    exit(-1);
  }

  //store label without ':'
  std::string label_name = split_line.at(0).substr(0, split_line.at(0).size() - 1);

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

void Parser::parseUType(Instruction &instruction, std::vector<std::string> &split_line)
{
  auto Rd = register_lut_.find(split_line.at(1));
  int imm = 0;

  if(Rd == register_lut_.end())
  {
    printf("Error with register parsing in UType\n");
    exit(-1);
  }

  getImm(imm, split_line.at(2));

  instruction.Rd_ = Rd->second;
  instruction.imm_ = imm;
}

void Parser::parseJType(Instruction &instruction, std::vector<std::string> &split_line)
{
  auto Rd = register_lut_.find(split_line.at(1));
  auto label = label_lut_.find(split_line.at(2));

  if(Rd == register_lut_.end())
  {
    printf("Error with register parsing in JAL\n");
    exit(-1);
  }

  if(label == label_lut_.end())
  {
    label_lut_.insert({split_line.at(2), {false}});
  }

  instruction.Rd_ = Rd->second;
  instruction.label_name_ = split_line.at(2);
}

void Parser::parseRType(Instruction &instruction, std::vector<std::string> &split_line)
{
  auto Rd = register_lut_.find(split_line.at(1));
  auto Rs1 = register_lut_.find(split_line.at(2));
  auto Rs2 = register_lut_.find(split_line.at(3));

  if(Rd == register_lut_.end() || Rs1 == register_lut_.end() || Rs2 == register_lut_.end())
  {
    printf("Error with register parsing in RType\n");
    exit(-1);
  }

  instruction.Rd_ = Rd->second;
  instruction.Rs1_ = Rs1->second;
  instruction.Rs2_ = Rs2->second;
}

void Parser::parseIType(Instruction &instruction, std::vector<std::string> &split_line)
{
  auto Rd = register_lut_.find(split_line.at(1));
  auto Rs1 = register_lut_.begin();
  int imm = 0;

  if(instruction.name_ == "ADDI" || instruction.name_ == "ANDI" || instruction.name_ == "SLLI" ||
    instruction.name_ == "SLTI" || instruction.name_ == "SLTIU" || instruction.name_ == "SRAI" ||
    instruction.name_ == "SRLI" || instruction.name_ == "XORI")
  {
    Rs1 = register_lut_.find(split_line.at(2));
    getImm(imm, split_line.at(3));
  }
  else
  {
    getImm(imm, split_line.at(2));
    Rs1 = register_lut_.find(split_line.at(3));
  }

  if(Rd == register_lut_.end() || Rs1 == register_lut_.end())
  {
    printf("Error with register parsing in IType\n");
    exit(-1);
  }

  instruction.Rd_ = Rd->second;
  instruction.Rs1_ = Rs1->second;
  instruction.imm_ = imm;
}

void Parser::parseSType(Instruction &instruction, std::vector<std::string> &split_line)
{
  auto Rs1 = register_lut_.find(split_line.at(1));
  auto Rs2 = register_lut_.find(split_line.at(3));
  int imm = 0;

  getImm(imm, split_line.at(2));

  if(Rs1 == register_lut_.end() || Rs2 == register_lut_.end())
  {
    printf("Error with register parsing in SType\n");
    exit(-1);
  }

  instruction.Rs1_ = Rs1->second;
  instruction.Rs2_ = Rs2->second;
  instruction.imm_ = imm;
}

void Parser::parseBType(Instruction &instruction, std::vector<std::string> &split_line)
{
  auto Rs1 = register_lut_.find(split_line.at(1));
  auto Rs2 = register_lut_.find(split_line.at(2));
  auto label = label_lut_.find(split_line.at(3));

  if(Rs1 == register_lut_.end() || Rs2 == register_lut_.end())
  {
    printf("Error with register parsing in BEQ/BNE/BLT/BGE\n");
    exit(-1);
  }

  if(label == label_lut_.end())
  {
    label_lut_.insert({split_line.at(3), {false}});
  }

  instruction.Rs1_ = Rs1->second;
  instruction.Rs2_ = Rs2->second;
  instruction.label_name_ = split_line.at(3);
}

bool Parser::isHex(std::string &line)
{
  if(line.find("0x") != line.npos)
  {
    return true;
  }

  return false;
}

void Parser::getImm(int &imm, std::string &line)
{
  if(isHex(line))
  {
    try
    {
      imm = std::stoi(line, 0, 16);
    }
    catch(std::exception &e)
    {
      printf("Error with immediate parsing: %s\n", e.what());
    }
  }
  else
  {
    try
    {
      imm = std::stoi(line);
    }
    catch(std::exception &e)
    {
      printf("Error with immediate parsing: %s\n", e.what());
    }
  }
}
