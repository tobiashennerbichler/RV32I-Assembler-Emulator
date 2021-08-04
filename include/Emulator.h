#include "CPU.h"

class Emulator
{
  public:
    void loadBinary();
    [[noreturn]] void run();

  private:
    CPU cpu_;
};