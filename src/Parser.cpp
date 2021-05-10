#include "../include/Parser.h"
#include <iostream>
#include <fstream>

Parser *Parser::instance_ = 0;

Parser *Parser::instance()
{
  if(!instance_)
    instance_ = new Parser();
  return instance_;
}

Parser::Parser()
{
  //use this look-up table to translate string representation of registers into its index
  lut_ = {
    {"zero", 0}, {"ra", 1}, {"sp", 2}, {"gp", 3}, {"tp", 4}, {"t0", 5},
    {"t1", 6}, {"t2", 7}, {"fp", 8}, {"s1", 9}, {"a0", 10}, {"a1", 11},
    {"a2", 12}, {"a3", 13}, {"a4", 14}, {"a5", 15}, {"a6", 16}, {"a7", 17},
    {"s2", 18}, {"s3", 19}, {"s4", 20}, {"s5", 21}, {"s6", 22}, {"s7", 23},
    {"s8", 24}, {"s9", 25}, {"s10", 26}, {"s11", 27}, {"t3", 28}, {"t4", 29},
    {"t5", 30}, {"t6", 31}
  };
}

Parser::~Parser()
{
  delete instance_;
}

/*
 * goes through the given file and separates each statement into instructions
 */
bool Parser::parse(std::vector<Instruction> &instructions, std::string filename)
{
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
      Instruction instruction = getInstruction(line);
      printf("%s, %d, %d, %d, %d\n", instruction.name_.c_str(), instruction.Rd_, instruction.Rs1_,
        instruction.Rs2_, instruction.imm_);
      instructions.push_back(instruction);
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
 * parses each line into instructions and checks for errors
 */
Instruction Parser::getInstruction(std::string line)
{
  Instruction instruction;
  std::vector<std::string> split_line = splitLine(line);

  if(split_line.empty())
  {
    printf("Invalid line (empty)\n");
    exit(-1);
  }

  instruction.name_ = split_line.at(0);

  //interpret single statements as labels with an ':' at the end as a label
  //only save label without ':' though
  if(split_line.size() == 1 && instruction.name_.back() == ':')
  {
    std::string label_name = split_line.at(0).substr(0, split_line.at(0).size() - 1);
    auto label = label_lut_.find(label_name);
    if(label == label_lut_.end())
    {
      printf("insert and verify label %s\n", label_name.c_str());
      label_lut_.insert({label_name, {label_count_++, true}});
    }
    else
    {
      printf("verify label %s\n", label_name.c_str());
      label->second.verified_ = true;
    }

    instruction.Rd_ = -1;
    instruction.Rs1_ = -1;
    instruction.Rs2_ = -1;
    instruction.imm_ = -1;

    return instruction;
  }

  //from here parse every supported instruction
  if(instruction.name_ == "EBREAK")
  {
    if(split_line.size() != 1)
    {
      printf("Invalid line\n");
      exit(-1);
    }

    instruction.Rd_ = -1;
    instruction.Rs1_ = -1;
    instruction.Rs2_ = -1;
    instruction.imm_ = -1;
  }
  else if(instruction.name_ == "JAL")
  {
    if(split_line.size() != 3)
    {
      printf("Invalid line\n");
      exit(-1);
    }

    parseJAL(instruction, split_line);
  }
  else
  {
    if(split_line.size() != 4)
    {
      printf("Invalid line\n");
      exit(-1);
    }

    if(instruction.name_ == "ADD" || instruction.name_ == "SUB")
    {
      parseADD(instruction, split_line);
    }
    else if(instruction.name_ == "LW" || instruction.name_ == "JALR")
    {
      parseLW(instruction, split_line);
    }
    else if(instruction.name_ == "SW")
    {
      parseSW(instruction, split_line);
    }
    else if(instruction.name_ == "ADDI")
    {
      parseADDI(instruction, split_line);
    }
    else if(instruction.name_ == "BEQ" || instruction.name_ == "BNE" ||
      instruction.name_ == "BLT" || instruction.name_ == "BGE")
    {
      parseBEQ(instruction, split_line);
    }
    else
    {
      printf("Instruction %s not known\n", instruction.name_.c_str());
      exit(-1);
    }
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

void Parser::parseJAL(Instruction &instruction, std::vector<std::string> &split_line)
{
  auto Rd = lut_.find(split_line.at(1));
  auto imm = label_lut_.find(split_line.at(2));

  if(Rd == lut_.end())
  {
    printf("Error with register parsing in JAL\n");
    exit(-1);
  }

  if(imm == label_lut_.end())
  {
    printf("insert label %s\n", split_line.at(2).c_str());
    label_lut_.insert({split_line.at(2), {label_count_++, false}});
    imm = label_lut_.find(split_line.at(2));
  }

  instruction.Rd_ = Rd->second;
  instruction.Rs1_ = -1;
  instruction.Rs2_ = -1;
  instruction.imm_ = imm->second.count_;
}

void Parser::parseADD(Instruction &instruction, std::vector<std::string> &split_line)
{
  auto Rd = lut_.find(split_line.at(1));
  auto Rs1 = lut_.find(split_line.at(2));
  auto Rs2 = lut_.find(split_line.at(3));

  if(Rd == lut_.end() || Rs1 == lut_.end() || Rs2 == lut_.end())
  {
    printf("Error with register parsing in ADD/SUB\n");
    exit(-1);
  }

  instruction.Rd_ = Rd->second;
  instruction.Rs1_ = Rs1->second;
  instruction.Rs2_ = Rs2->second;
  instruction.imm_ = -1;
}

void Parser::parseLW(Instruction &instruction, std::vector<std::string> &split_line)
{
  auto Rd = lut_.find(split_line.at(1));
  auto Rs1 = lut_.find(split_line.at(3));
  int imm = 0;

  if(isHex(split_line.at(2)))
  {
    try
    {
      imm = std::stoi(split_line.at(2), 0, 16);
    }
    catch(std::exception &e)
    {
      printf("Error with register parsing in LW/JALR: %s\n", e.what());
      exit(-1);
    }
  }
  else
  {
    try
    {
      imm = std::stoi(split_line.at(2));
    }
    catch(std::exception &e)
    {
      printf("Error with register parsing in LW/JALR: %s\n", e.what());
      exit(-1);
    }
  }

  if(Rd == lut_.end() || Rs1 == lut_.end())
  {
    printf("Error with register parsing in LW/JALR\n");
    exit(-1);
  }

  instruction.Rd_ = Rd->second;
  instruction.Rs1_ = Rs1->second;
  instruction.Rs2_ = -1;
  instruction.imm_ = imm;
}

void Parser::parseSW(Instruction &instruction, std::vector<std::string> &split_line)
{
  auto Rs1 = lut_.find(split_line.at(1));
  auto Rs2 = lut_.find(split_line.at(3));
  int imm = 0;

  if(isHex(split_line.at(2)))
  {
    try
    {
      imm = std::stoi(split_line.at(2), 0, 16);
    }
    catch(std::exception &e)
    {
      printf("Error with register parsing in SW: %s\n", e.what());
      exit(-1);
    }
  }
  else
  {
    try
    {
      imm = std::stoi(split_line.at(2));
    }
    catch(std::exception &e)
    {
      printf("Error with register parsing in SW: %s\n", e.what());
      exit(-1);
    }
  }

  if(Rs1 == lut_.end() || Rs2 == lut_.end())
  {
    printf("Error with register parsing in SW\n");
    exit(-1);
  }

  instruction.Rd_ = -1;
  instruction.Rs1_ = Rs1->second;
  instruction.Rs2_ = Rs2->second;
  instruction.imm_ = imm;
}

void Parser::parseADDI(Instruction &instruction, std::vector<std::string> &split_line)
{
  auto Rd = lut_.find(split_line.at(1));
  auto Rs1 = lut_.find(split_line.at(2));
  int imm = 0;

  if(isHex(split_line.at(3)))
  {
    try
    {
      imm = std::stoi(split_line.at(3), 0, 16);
    }
    catch(std::exception &e)
    {
      printf("Error with register parsing in ADDI: %s\n", e.what());
    }
  }
  else
  {
    try
    {
      imm = std::stoi(split_line.at(3));
    }
    catch(std::exception &e)
    {
      printf("Error with register parsing in ADDI: %s\n", e.what());
    }
  }

  if(Rd == lut_.end() || Rs1 == lut_.end())
  {
    printf("Error with register parsing in ADDI\n");
    exit(-1);
  }

  instruction.Rd_ = Rd->second;
  instruction.Rs1_ = Rs1->second;
  instruction.Rs2_ = -1;
  instruction.imm_ = imm;
}

void Parser::parseBEQ(Instruction &instruction, std::vector<std::string> &split_line)
{
  auto Rs1 = lut_.find(split_line.at(1));
  auto Rs2 = lut_.find(split_line.at(2));
  auto imm = label_lut_.find(split_line.at(3));

  if(Rs1 == lut_.end() || Rs2 == lut_.end())
  {
    printf("Error with register parsing in BEQ/BNE/BLT/BGE\n");
    exit(-1);
  }

  if(imm == label_lut_.end())
  {
    printf("insert label %s\n", split_line.at(3).c_str());
    label_lut_.insert({split_line.at(3), {label_count_++, false}});
    imm = label_lut_.find(split_line.at(3));
  }

  instruction.Rd_ = -1;
  instruction.Rs1_ = Rs1->second;
  instruction.Rs2_ = Rs2->second;
  instruction.imm_ = imm->second.count_;
}

bool Parser::isHex(std::string &line)
{
  if(line.find("0x") != line.npos)
  {
    return true;
  }

  return false;
}
