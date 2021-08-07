#include "../include/Emulator.h"
#include <fstream>

Emulator::Emulator()
{
  initWindow();

  for(int y = 0; y < HEIGHT; y++)
  {
    std::vector<int> pixels;

    for(int x = 0; x < WIDTH; x++)
    {
      pixels.push_back(GREY);
    }

    field_.push_back(pixels);
  }
}

void Emulator::loadBinary()
{
  u32 address = 0;
  std::fstream file = std::fstream("../memory.bin", std::ios::in | std::ios::binary);

  while(!file.eof())
  {
    u8 byte;
    file.read((char *) &byte, sizeof(u8));
    cpu_.write(address++, byte);
  }
}

[[noreturn]] void Emulator::run()
{
  while(true)
  {
    cpu_.tick();
    draw();

    if(!window_open_)
    {
      exit(0);
    }
  }
}

void Emulator::initWindow()
{
  if(SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    printf("Failed to initialize the SDL2 library\n");
    exit(-1);
  }

  window_ = SDL_CreateWindow("RISC-V RV32I Emulator",
    SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WIDTH*PIXEL_SIZE, HEIGHT*PIXEL_SIZE, SDL_WINDOW_OPENGL);

  if(!window_)
  {
    printf("Failed to create window\n");
    exit(-1);
  }

  renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if(!renderer_)
  {
    printf("Renderer could not be created\n");
    exit(-1);
  }

  texture_ = SDL_CreateTexture(
    renderer_,
    SDL_PIXELFORMAT_ARGB8888,
    SDL_TEXTUREACCESS_STREAMING,
    WIDTH*PIXEL_SIZE, HEIGHT*PIXEL_SIZE
  );

  if(!texture_)
  {
    printf("Texture could not be acquired\n");
    exit(-1);
  }
}

void Emulator::executeEvents()
{
  SDL_Event event;

  while(SDL_PollEvent(&event))
  {
    switch(event.type)
    {
      case SDL_QUIT:
        window_open_ = 0;
        break;
      case SDL_KEYDOWN:
        if(event.key.keysym.sym == SDLK_x)
        {
          add0(25, 25);
          continue_ = 0;
        }
        if(event.key.keysym.sym == SDLK_y)
        {
          add1(0, 0);
          continue_ = 0;
        }
        break;
    }

    SDL_UpdateWindowSurface(window_);
  }
}

void Emulator::draw()
{
  while(continue_)
  {
    executeEvents();
  }

  continue_ = 1;

  SDL_RenderClear(renderer_);

  void* pixels_ptr;
  int pitch;
  SDL_LockTexture(texture_, 0, &pixels_ptr, &pitch);

  uint32_t *pixels = (uint32_t *) pixels_ptr;

  for(int y = 0; y < HEIGHT; y++)
  {
    for(int x = 0; x < WIDTH; x++)
    {
      for (uint w = 0; w < PIXEL_SIZE; w++)
      {
        for (uint h = 0; h < PIXEL_SIZE; h++)
        {
          pixels[WIDTH * PIXEL_SIZE * (y * PIXEL_SIZE + h) + (x * PIXEL_SIZE + w)] = field_.at(y).at(x);
        }
      }
    }
  }

  SDL_UnlockTexture(texture_);

  SDL_RenderCopy(renderer_, texture_, 0, 0);
  SDL_RenderPresent(renderer_);
}

void Emulator::add0(int x, int y)
{
  for(int i = 1; i <= 3; i++)
  {
    field_.at(y).at(x + i) = WHITE;
    field_.at(y + 6).at(x + i) = WHITE;
  }

  for(int i = 1; i <= 5; i++)
  {
    field_.at(y + i).at(x) = WHITE;
    field_.at(y + i).at(x + 4) = WHITE;
  }
}

void Emulator::add1(int x, int y)
{
  for(int i = 1; i < 3; i++)
  {
    field_.at(y).at(x + i) = WHITE;
  }

  for(int i = 1; i <= 6; i++)
  {
    field_.at(y + i).at(x + 2) = WHITE;
  }

  for(int i = 0; i < 5; i++)
  {
    field_.at(y + 6).at(x + i) = WHITE;
  }
}

void Emulator::add2(int x, int y)
{
  field_.at(y + 1).at(x) = WHITE;
  field_.at(y + 1).at(x + 4) = WHITE;

  for(int i = 1; i <= 3; i++)
  {
    field_.at(y).at(x + i) = WHITE;
  }

  for(int i = 0; i < 5; i++)
  {
    field_.at(y + 6).at(x + i) = WHITE;
  }
}
