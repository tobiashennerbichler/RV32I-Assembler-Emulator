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
    void initPatterns(int elements, int elements_per_row, std::string &file_name, std::vector<Pattern> &patterns);
    void initRegisterNames();

    [[noreturn]] void run();

    void executeEvents();
    void draw();
    void drawPattern(int x, int y, Pattern &pattern, Highlight highlight);
    Highlight getHighlight(u32 address);
    void updateScreen();
    void resetScreen();
    void drawMemorySection(int x, int y, u32 start_address, u32 end_address);
    void drawRegisters(int x, int y);
    void drawString(int x, int y, std::string string);
    u32 getAsciiNumber(u32 number);

    CPU cpu_;
    SDL_Window *window_;
    SDL_Renderer *renderer_;
    SDL_Texture *texture_;
    int continue_ = 1;
    u32 code_size_ = 0;
    std::vector<Pattern> ascii_patterns_;
    std::vector<std::string> register_names_;
    std::vector<std::vector<int>> field_;
    std::vector<u8> saved_memory_;
    u32 saved_registers_[32];
};