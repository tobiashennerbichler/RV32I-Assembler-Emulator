#include "../include/Assembler.h"
#include "../include/Emulator.h"

int main(int argc, char *argv[])
{
  //TODO: make two mains, one for assembling, the other for emulating
  if(argc != 2)
  {
    printf("Usage: ./binary [filename]\n");
    exit(-1);
  }

  std::string file_name = argv[1];
  Assembler assembler;
  assembler.assemble(file_name);
  Emulator emulator;
  emulator.loadBinary();
  emulator.run();

  return 0;
}
