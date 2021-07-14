#include "Parser.h"
#include <vector>

class CPU
{
  public:
    CPU();
    virtual ~CPU();
    u32 read(u32 address);
    void write(u32 address, u32 data);

  private:
    u32 x_[32];
    u32 pc_;
    std::vector<u32> memory_;
};
