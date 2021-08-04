#include <iostream>
#include "../include/Assembler.h"
#include "../include/Emulator.h"

int main()
{
  std::string file_name = "../FileTests/test0.txt";
  Assembler assembler;
  assembler.assemble(file_name);
  Emulator emulator;
  emulator.loadBinary();
  emulator.run();

  return 0;
}
