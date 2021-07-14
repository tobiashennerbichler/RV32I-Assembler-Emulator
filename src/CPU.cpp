#include "../include/CPU.h"

CPU::CPU() : memory_(0x800)
{

}

CPU::~CPU()
{

}

u32 CPU::read(u32 address)
{
  return memory_.at(address);
}

void CPU::write(u32 address, u32 data)
{
  memory_.at(address) = data;
}