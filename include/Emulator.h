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
    void add0(int x, int y);
    void add1(int x, int y);
    void add2(int x, int y);
    void add3(int x, int y);
    void add4(int x, int y);
    void add5(int x, int y);
    void add6(int x, int y);
    void add7(int x, int y);
    void add8(int x, int y);
    void add9(int x, int y);
    void addA(int x, int y);
    void addB(int x, int y);
    void addC(int x, int y);
    void addD(int x, int y);
    void addE(int x, int y);
    void addF(int x, int y);
    CPU cpu_;
    SDL_Window *window_;
    SDL_Renderer *renderer_;
    SDL_Texture *texture_;
    int window_open_ = 1;
    int continue_ = 1;
    std::vector<std::vector<int>> field_;
};