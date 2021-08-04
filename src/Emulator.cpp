#include "../include/Emulator.h"
#include <fstream>

void Emulator::loadBinary()
{
  u32 address = 0;
  std::fstream file = std::fstream("../memory.bin", std::ios::in | std::ios::binary);

  while(!file.eof())
  {
    u8 byte;
    file.read((char *) &byte, sizeof(u8));
    cpu_.write(address++, byte);
  }
}

[[noreturn]] void Emulator::run()
{
  while(true)
  {
    cpu_.tick();
  }
}
