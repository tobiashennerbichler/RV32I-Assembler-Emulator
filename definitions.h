#include <cstdint>

typedef uint32_t u32;
typedef int32_t s32;

struct Instruction
{
  std::string name_;
  int Rd_;
  int Rs1_;
  int Rs2_;
  int imm_;
};