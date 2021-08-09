#include "CPU.h"
#include "SDL.h"

class Emulator
{
  public:
    Emulator();
    void loadBinary();

  private:
    void initWindow();
    void initField();
    void initPatterns(int elements_per_row, int elements_per_col, std::string &file_name, std::vector<Pattern> &patterns);
    void initRegisterPatterns();
    void checkScreenBoundaries();

    [[noreturn]] void run();

    void executeEvents();
    void draw();
    void drawPattern(int x, int y, Pattern &pattern);
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
    std::vector<Pattern> numbers_;
    std::vector<Pattern> register_names_;
    std::vector<std::vector<int>> field_;
    std::vector<u8> saved_memory_;
    u32 saved_registers_[32];
};