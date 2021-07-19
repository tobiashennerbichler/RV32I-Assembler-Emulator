#include <iostream>
#include "../include/Assembler.h"

int main()
{
  std::string file_name = "../RISC-V-Interpreter/FileTests/test0.txt";
  CPU cpu;
  Assembler assembler(cpu);
  assembler.assemble(file_name);

  return 0;
}
