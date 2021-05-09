#include "Parser.h"
#include <iostream>
#include <fstream>

Parser::Parser()
{
  lut_ = {
    {"zero", 0}, {"ra", 1}, {"sp", 2}, {"gp", 3}, {"tp", 4}, {"t0", 5},
    {"t1", 6}, {"t2", 7}, {"fp", 8}, {"s1", 9}, {"a0", 10}, {"a1", 11},
    {"a2", 12}, {"a3", 13}, {"a4", 14}, {"a5", 15}, {"a6", 16}, {"a7", 17},
    {"s2", 18}, {"s3", 19}, {"s4", 20}, {"s5", 21}, {"s6", 22}, {"s7", 23},
    {"s8", 24}, {"s9", 25}, {"s10", 26}, {"s11", 27}, {"t3", 28}, {"t4", 29},
    {"t5", 30}, {"t6", 31}
  };
}

void Parser::parse(std::vector<Instruction> &instructions, std::string filename)
{
  file_ = std::fstream(filename, std::ios::in);

  std::string line;
  while(std::getline(file_, line))
  {
    std::cout << line << std::endl;
    Instruction instruction = getInstruction(line);
    printf("%s, %d, %d, %d, %d\n", instruction.name_.c_str(), instruction.Rd_, instruction.Rs1_,
      instruction.Rs2_, instruction.imm_);
  }


}

Instruction Parser::getInstruction(std::string line)
{
  Instruction instruction;
  std::vector<std::string> split_line = splitLine(line);

  instruction.name_ = split_line.at(0);

  if(instruction.name_ == "ADD" || instruction.name_ == "SUB")
  {
    auto Rd = lut_.find(split_line.at(1));
    auto Rs1 = lut_.find(split_line.at(2));
    auto Rs2 = lut_.find(split_line.at(3));

    if(Rd == lut_.end() || Rs1 == lut_.end() || Rs2 == lut_.end())
    {
      printf("Error in ADD or SUB with register parsing\n");
      exit(-1);
    }

    instruction.Rd_ = Rd->second;
    instruction.Rs1_ = Rs1->second;
    instruction.Rs2_ = Rs2->second;
    instruction.imm_ = -1;
  }
  else if(instruction.name_ == "ADDI" || instruction.name_ == "LW" ||
    instruction.name_ == "JALR")
  {
    auto Rd = lut_.find(split_line.at(1));
    auto Rs1 = lut_.find(split_line.at(2));
    int imm = 0;
    try
    {
      imm = std::stoi(split_line.at(3));
    }
    catch(std::exception &e)
    {
      printf("Exception in ADDI/JW/JALR: %s\n", e.what());
      exit(-1);
    }

    if(Rd == lut_.end() || Rs1 == lut_.end())
    {
      printf("Error in ADDI or LW with register parsing\n");
      exit(-1);
    }

    instruction.Rd_ = Rd->second;
    instruction.Rs1_ = Rs1->second;
    instruction.Rs2_ = -1;
    instruction.imm_ = imm;
  }
  else if(instruction.name_ == "SW" || instruction.name_ == "BEQ" ||
    instruction.name_ == "BNE" || instruction.name_ == "BLT" ||
    instruction.name_ == "BGE")
  {
    auto Rs1 = lut_.find(split_line.at(1));
    auto Rs2 = lut_.find(split_line.at(2));
    int imm = 0;
    try
    {
      imm = std::stoi(split_line.at(3));
    }
    catch(std::exception &e)
    {
      printf("Exception in SW/BEQ/BNE/BLT/BGE: %s\n", e.what());
      exit(-1);
    }

    instruction.Rd_ = -1;
    instruction.Rs1_ = Rs1->second;
    instruction.Rs2_ = Rs2->second;
    instruction.imm_ = imm;
  }
  else if(instruction.name_ == "JAL")
  {
    auto Rd = lut_.find(split_line.at(1));
    int imm = 0;

    try
    {
      imm = std::stoi(split_line.at(2));
    }
    catch(std::exception &e)
    {
      printf("Exception in JAL: %s\n", e.what());
      exit(-1);
    }

    instruction.Rd_ = Rd->second;
    instruction.Rs1_ = -1;
    instruction.Rs2_ = -1;
    instruction.imm_ = imm;
  }

  return instruction;
}

std::vector<std::string> Parser::splitLine(std::string &line)
{
  std::vector<std::string> attributes;
  line.push_back(' ');
  std::string split;

  for(auto &s : line)
  {
    if(s == ' ' || s == ',')
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
