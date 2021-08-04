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

  for(u32 addr = 0; addr < (address - 1); addr++)
  {
    printf("0x%x ", cpu_.read(addr));
  }
}
