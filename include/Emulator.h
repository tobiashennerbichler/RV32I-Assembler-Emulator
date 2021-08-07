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
    void executeEvents();
    void draw();
    void addNumber(int x, int y, int number, bool highlight);
    void updateScreen();
    CPU cpu_;
    SDL_Window *window_;
    SDL_Renderer *renderer_;
    SDL_Texture *texture_;
    int window_open_ = 1;
    int continue_ = 1;
    std::vector<std::vector<std::pair<int, int>>> number_patterns_;
    std::vector<std::vector<int>> field_;
};