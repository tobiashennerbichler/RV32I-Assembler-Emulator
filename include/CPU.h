#include "Parser.h"
#include <vector>

class CPU
{
  public:
    CPU();
    virtual ~CPU();
    u8 read(u32 address);
    void write(u32 address, u8 data);

  private:
    u32 registers_[32];
    u32 pc_;
    std::vector<u8> memory_;
};
