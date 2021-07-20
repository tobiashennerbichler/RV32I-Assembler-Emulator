#include "../include/CPU.h"

CPU::CPU() : memory_(0x10000)
{

}

CPU::~CPU()
{

}

u8 CPU::read(u32 address)
{
  return memory_.at(address);
}

void CPU::write(u32 address, u8 data)
{
  memory_.at(address) = data;
}