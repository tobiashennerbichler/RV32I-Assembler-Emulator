#include <string>
#include <vector>
#include "../include/TestSystem.h"
#include "../include/Parser.h"

bool TestSystem::testFiles()
{
  for(int i = 0; i < 1; i++)
  {
    std::string file_name = "test" + std::to_string(i) + ".txt";
    std::string ref_file_name = "test" + std::to_string(i) + "_ref.txt";

    if(testFile(file_name, ref_file_name))
    {
      printf("%s finished successfully\n", file_name.c_str());
    }
    else
    {
      printf("%s failed\n", file_name.c_str());
    }
  }

  return true;
}

bool TestSystem::testFile(std::string &file_name, std::string &ref_file_name)
{
  std::vector<Instruction> instructions;
  if(!Parser::instance()->parse(instructions, "../RISC-V-Interpreter/FileTests/" + file_name))
  {
    return false;
  }

  std::ifstream ref_file = std::ifstream("../RISC-V-Interpreter/FileRef/" + ref_file_name);

  if(!ref_file.is_open())
  {
    printf("file %s could not be opened\n", ref_file_name.c_str());
    return false;
  }

  auto instruction = instructions.begin();
  std::string ref_line;

  while(std::getline(ref_file, ref_line))
  {
//    std::string line = instruction->name_ + " " + std::to_string(instruction->Rd_) + " " +
//      std::to_string(instruction->Rs1_) + " " + std::to_string(instruction->Rs2_) + " " +
//      std::to_string(instruction->imm_);
    printf("line: %s %d %d %d %d\n", instruction->name_.c_str(), instruction->Rd_, instruction->Rs1_,
      instruction->Rs2_, instruction->imm_);
  }

  return true;
}
