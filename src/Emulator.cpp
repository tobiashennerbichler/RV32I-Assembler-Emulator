#include "../include/Emulator.h"
#include <fstream>

Emulator::Emulator()
{
  for(int y = 0; y < HEIGHT; y++)
  {
    std::vector<int> pixels;

    for(int x = 0; x < WIDTH; x++)
    {
      pixels.push_back(GREY);
    }

    field_.push_back(pixels);
  }

  //TODO: make general for dynamic size
  number_patterns_ = {
      {
        {0, 1}, {0, 2}, {0, 3}, {6, 1}, {6, 2}, {6, 3}, {1, 0}, {2, 0},
        {3, 0}, {4, 0}, {5, 0}, {1, 4}, {2, 4}, {3, 4}, {4, 4}, {5, 4}
      },
      {
        {0, 1}, {0, 2}, {1, 2}, {2, 2}, {3, 2}, {4, 2}, {5, 2}, {6, 2},
        {6, 0}, {6, 1}, {6, 2}, {6, 3}, {6, 4}
      },
      {
        {1, 0}, {1, 4}, {0, 1}, {0, 2}, {0, 3}, {6, 0}, {6, 1}, {6, 2},
        {6, 3}, {6, 4}, {5, 0}, {4, 1}, {3, 2}, {2, 3}
      },
      {
        {1, 0}, {0, 1}, {0, 2}, {0, 3}, {1, 4}, {2, 4}, {3, 3}, {3, 2},
        {3, 1}, {5, 0}, {6, 1}, {6, 2}, {6, 3}, {5, 4}, {4, 4}
      },
      {
        {0, 0}, {1, 0}, {2, 0}, {3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4},
        {1, 3}, {2, 3}, {4, 3}, {5, 3}, {6, 3}
      },
      {
        {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 0}, {2, 0}, {3, 0},
        {3, 1}, {3, 2}, {3, 3}, {3, 4}, {4, 4}, {5, 4}, {6, 4}, {6, 3},
        {6, 2}, {6, 1}, {6, 0}
      },
      {
        {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 0}, {2, 0}, {3, 0},
        {4, 0}, {5, 0}, {6, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4}, {4, 4},
        {5, 4}, {6, 4}, {6, 3}, {6, 2}, {6, 1}
      },
      {
        {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 4}, {2, 3}, {3, 3},
        {4, 2}, {5, 2}, {6, 1}
      },
      {
        {0, 1}, {0, 2}, {0, 3}, {1, 0}, {2, 0}, {3, 1}, {3, 2}, {3, 3},
        {2, 4}, {1, 4}, {4, 0}, {5, 0}, {6, 1}, {6, 2}, {6, 3}, {5, 4},
        {4, 4}
      },
      {
        {0, 1}, {0, 2}, {0, 3}, {1, 0}, {2, 0}, {3, 1}, {3, 2}, {3, 3},
        {2, 4}, {1, 4}, {3, 4}, {4, 4}, {5, 4}, {6, 3}, {6, 2}, {6, 1},
        {5, 0}
      },
      {
        {0, 1}, {0, 2}, {0, 3}, {1, 0}, {2, 0}, {3, 0}, {3, 1}, {3, 2},
        {3, 3}, {3, 4}, {2, 4}, {1, 4}, {4, 0}, {5, 0}, {6, 0}, {4, 4},
        {5, 4}, {6, 4}
      },
      {
        {0, 0}, {0, 1}, {0, 2}, {0, 3}, {1, 0}, {2, 0}, {3, 0}, {4, 0},
        {5, 0}, {6, 0}, {6, 1}, {3, 1}, {3, 2}, {3, 3}, {2, 4}, {1, 4},
        {6, 2}, {6, 3}, {5, 4}, {4, 4}
      },
      {
        {0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 0}, {2, 0}, {3, 0}, {4, 0},
        {5, 0}, {6, 1}, {6, 2}, {6, 3}, {5, 4}
      },
      {
        {0, 0}, {0, 1}, {0, 2}, {0, 3}, {1, 0}, {2, 0}, {3, 0}, {4, 0},
        {5, 0}, {6, 0}, {6, 1}, {6, 2}, {6, 3}, {1, 4}, {2, 4}, {3, 4},
        {4, 4}, {5, 4}
      },
      {
        {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 0}, {2, 0}, {3, 0},
        {4, 0}, {5, 0}, {6, 0}, {3, 1}, {3, 2}, {3, 3}, {6, 1}, {6, 2},
        {6, 3}, {6, 4}
      },
      {
        {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 0}, {2, 0}, {3, 0},
        {3, 1}, {3, 2}, {3, 3}, {4, 0}, {5, 0}, {6, 0}
      }
  };
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
  initWindow();
  updateScreen();
  draw();

  while(true)
  {
    cpu_.tick();

    while(continue_)
    {
      executeEvents();
    }

    continue_ = 1;

    draw();
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
        exit(0);
      case SDL_KEYDOWN:
        if(event.key.keysym.sym == SDLK_x)
        {
          updateScreen();
          continue_ = 0;
        }
        break;
    }

    SDL_UpdateWindowSurface(window_);
  }
}

void Emulator::draw()
{
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

void Emulator::addNumber(int x, int y, u8 number, bool highlight)
{
  auto pattern = number_patterns_.at(number);

  for(auto &p : pattern)
  {
    if(highlight)
    {
      field_.at(y + p.first).at(x + p.second) = BLUE;
    }
    else
    {
      field_.at(y + p.first).at(x + p.second) = WHITE;
    }
  }
}

void Emulator::updateScreen()
{
  int x = 1, y = 1;
  int width = 5;
  int height = 7;

  for(int i = 0; i < HEIGHT; i++)
  {
    for(int j = 0; j < WIDTH; j++)
    {
      field_.at(i).at(j) = GREY;
    }
  }

  for(u32 address = 0; address < 0x100; address++)
  {
    if(x == 1)
    {
      for(int i = 3; i >= 0; i--)
      {
        addNumber(x, y, (address >> (i*8 + 4)) & 0xF, false);
        x += width + 1;
        addNumber(x, y, (address >> i*8) & 0xF, false);
        x += 2*width;
      }

      x += 4*width;
    }

    bool highlight = (address >= cpu_.getPC() && address < (cpu_.getPC() + 4));

    u8 byte = cpu_.read(address);
    addNumber(x, y, (byte >> 4) & 0xF, highlight);
    x += width + 1;
    addNumber(x, y, byte & 0xF, highlight);
    x += 2*width;

    if(((address + 1) % 16) == 0)
    {
      y += height + 2;
      x = 1;
    }
  }
}

