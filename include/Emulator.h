#include "CPU.h"
#include "SDL.h"

class Emulator
{
  public:
    Emulator();
    void loadBinary();
    [[noreturn]] void run();

  private:
    void initWindow();
    void initField();
    void initNumberPatterns();
    void initRegisterNamePatterns();

    void executeEvents();
    void draw();
    void drawPattern(int x, int y, std::vector<std::pair<int, int>> &pattern, Highlight highlight);
    Highlight getHighlight(u32 address);
    void updateScreen();
    void resetScreen();
    void drawMemorySection(int x, int y, u32 start_address, u32 end_address);
    void drawRegisters();

    CPU cpu_;
    SDL_Window *window_;
    SDL_Renderer *renderer_;
    SDL_Texture *texture_;
    int continue_ = 1;
    u32 code_size_ = 0;
    std::vector<std::vector<std::pair<int, int>>> number_patterns_;
    std::vector<std::vector<std::pair<int, int>>> register_name_patterns_;
    std::vector<std::vector<int>> field_;
    std::vector<u8> saved_memory_;
    u32 saved_registers_[32];
};