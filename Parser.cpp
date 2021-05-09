#include "Parser.h"
#include <iostream>
#include <fstream>

Parser::Parser()
{

}

void Parser::parse(std::vector<Instruction> &instructions, std::string filename)
{
  file_ = std::fstream(filename, std::ios::in);

  std::string line;
  while(std::getline(file_, line))
  {
    std::cout << line << std::endl;
  }


}

Instruction Parser::getInstruction(std::string line)
{

}
