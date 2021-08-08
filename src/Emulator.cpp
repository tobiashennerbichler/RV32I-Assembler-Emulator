#include "../include/Emulator.h"
#include <fstream>
#include <cassert>

Emulator::Emulator()
{
  initWindow();
  initField();
  initNumberPatterns();
  initRegisterNamePatterns();
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

  code_size_ = address - 1;
  saved_memory_ = cpu_.getMemory();

  for(int i = 0; i < 32; i++)
  {
    saved_registers_[i] = cpu_.getRegister(i);
  }
}

[[noreturn]] void Emulator::run()
{
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

    saved_memory_ = cpu_.getMemory();

    for(int i = 0; i < 32; i++)
    {
      saved_registers_[i] = cpu_.getRegister(i);
    }

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

void Emulator::initField()
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
}

void Emulator::initNumberPatterns()
{
  //TODO: make it general for dynamic size
  number_patterns_ = {
    //0
    {
      {0, 1}, {0, 2}, {0, 3}, {6, 1}, {6, 2}, {6, 3}, {1, 0}, {2, 0},
      {3, 0}, {4, 0}, {5, 0}, {1, 4}, {2, 4}, {3, 4}, {4, 4}, {5, 4}
    },
    //1
    {
      {0, 1}, {0, 2}, {1, 2}, {2, 2}, {3, 2}, {4, 2}, {5, 2}, {6, 2},
      {6, 0}, {6, 1}, {6, 2}, {6, 3}, {6, 4}
    },
    //2
    {
      {1, 0}, {1, 4}, {0, 1}, {0, 2}, {0, 3}, {6, 0}, {6, 1}, {6, 2},
      {6, 3}, {6, 4}, {5, 0}, {5, 1}, {4, 1}, {4, 2}, {3, 2}, {3, 3},
      {2, 3}, {2, 4}
    },
    //3
    {
      {1, 0}, {0, 1}, {0, 2}, {0, 3}, {1, 4}, {2, 4}, {3, 3}, {3, 2},
      {3, 1}, {5, 0}, {6, 1}, {6, 2}, {6, 3}, {5, 4}, {4, 4}
    },
    //4
    {
      {0, 0}, {1, 0}, {2, 0}, {3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4},
      {1, 3}, {2, 3}, {4, 3}, {5, 3}, {6, 3}
    },
    //5
    {
      {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 0}, {2, 0}, {3, 0},
      {3, 1}, {3, 2}, {3, 3}, {3, 4}, {4, 4}, {5, 4}, {6, 4}, {6, 3},
      {6, 2}, {6, 1}, {6, 0}
    },
    //6
    {
      {0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 0}, {2, 0}, {3, 0}, {4, 0},
      {5, 0}, {3, 1}, {3, 2}, {3, 3}, {4, 4}, {5, 4}, {6, 3}, {6, 2},
      {6, 1}
    },
    //7
    {
      {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 4}, {2, 4}, {3, 3},
      {4, 2}, {5, 1}, {6, 0}
    },
    //8
    {
      {0, 1}, {0, 2}, {0, 3}, {1, 0}, {2, 0}, {3, 1}, {3, 2}, {3, 3},
      {2, 4}, {1, 4}, {4, 0}, {5, 0}, {6, 1}, {6, 2}, {6, 3}, {5, 4},
      {4, 4}
    },
    //9
    {
      {0, 1}, {0, 2}, {0, 3}, {1, 0}, {2, 0}, {3, 1}, {3, 2}, {3, 3},
      {2, 4}, {1, 4}, {3, 4}, {4, 4}, {5, 4}, {6, 3}, {6, 2}, {6, 1},
      {5, 0}
    },
    //A
    {
      {0, 1}, {0, 2}, {0, 3}, {1, 0}, {2, 0}, {3, 0}, {3, 1}, {3, 2},
      {3, 3}, {3, 4}, {2, 4}, {1, 4}, {4, 0}, {5, 0}, {6, 0}, {4, 4},
      {5, 4}, {6, 4}
    },
    //B
    {
      {0, 0}, {0, 1}, {0, 2}, {0, 3}, {1, 0}, {2, 0}, {3, 0}, {4, 0},
      {5, 0}, {6, 0}, {6, 1}, {3, 1}, {3, 2}, {3, 3}, {2, 4}, {1, 4},
      {6, 2}, {6, 3}, {5, 4}, {4, 4}
    },
    //C
    {
      {0, 1}, {0, 2}, {0, 3}, {1, 4}, {1, 0}, {2, 0}, {3, 0}, {4, 0},
      {5, 0}, {6, 1}, {6, 2}, {6, 3}, {5, 4}
    },
    //D
    {
      {0, 0}, {0, 1}, {0, 2}, {0, 3}, {1, 0}, {2, 0}, {3, 0}, {4, 0},
      {5, 0}, {6, 0}, {6, 1}, {6, 2}, {6, 3}, {1, 4}, {2, 4}, {3, 4},
      {4, 4}, {5, 4}
    },
    //E
    {
      {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 0}, {2, 0}, {3, 0},
      {4, 0}, {5, 0}, {6, 0}, {3, 1}, {3, 2}, {3, 3}, {6, 1}, {6, 2},
      {6, 3}, {6, 4}
    },
    //F
    {
      {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 0}, {2, 0}, {3, 0},
      {3, 1}, {3, 2}, {3, 3}, {4, 0}, {5, 0}, {6, 0}
    }
  };
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
          return;
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
      for(int w = 0; w < PIXEL_SIZE; w++)
      {
        for(int h = 0; h < PIXEL_SIZE; h++)
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

void Emulator::drawPattern(int x, int y, std::vector<std::pair<int, int>> &pattern, Highlight highlight)
{
  for(auto &p : pattern)
  {
    field_.at(y + p.first).at(x + p.second) = highlight;
  }
}

Highlight Emulator::getHighlight(u32 address)
{
  Highlight highlight;

  if(address >= cpu_.getPC() && address < (cpu_.getPC() + 4))
  {
    highlight = PC;
  }
  else if(saved_memory_.at(address) != cpu_.read(address))
  {
    highlight = CHANGED;
  }
  else
  {
    highlight = NONE;
  }

  return highlight;
}

//TODO: make it more adaptive to screen size
/*
 * draw memory section around pc, around sp and draw registers to the screen
 * calculate where the positions are, depending on the defines
 */
void Emulator::updateScreen()
{
  int x = 1, y = 1;

  resetScreen();

  u32 start = cpu_.getPC() - 0x50;
  u32 end = cpu_.getPC() + 0x50;
  drawMemorySection(x, y, start, end);

  y += ((int) ((end - start) / BYTE_PER_LINE) + 2) * ROW_DISTANCE;
  start = cpu_.getRegister(2) - 0x80;
  end = cpu_.getRegister(2) + 0x80;
  drawMemorySection(x, y, start, end);

  drawRegisters();
}

/*
 * resets the whole screen by setting each pixel to grey
 */
void Emulator::resetScreen()
{
  for(int i = 0; i < HEIGHT; i++)
  {
    for(int j = 0; j < WIDTH; j++)
    {
      field_.at(i).at(j) = GREY;
    }
  }
}

/*
 * draws (end_address - start_address) bytes at position x and y to the screen
 * checks if bytes will fit on screen and otherwise assert
 * if start_address < 0 then we set start_address to 0 and add the rest to end_address to get the same range
 * if start_address not aligned on the bytes per line, then align it and end_address to get the same range
 * then first draw address of the current line and then each byte per line
 */
//TODO: some more general checking if everything can fit on the screen at the start of the emulator
void Emulator::drawMemorySection(int x, int y, u32 start_address, u32 end_address)
{
  assert((s32) start_address < (s32) end_address && "start address >= end address");
  assert(x >= 0 && y >= 0 && "x or y < 0");

  //there are no entries at addresses < 0
  if((s32) start_address < 0)
  {
    end_address -= start_address;
    start_address = 0;
  }

  //if start address not aligned, align it
  if((start_address % BYTE_PER_LINE) != 0)
  {
    end_address -= start_address % BYTE_PER_LINE;
    start_address -= start_address % BYTE_PER_LINE;
  }

  u32 address = start_address;
  u32 size = end_address - start_address;

  u32 max_x = x + (X_LEN / 8) * (HI_LO_DISTANCE + BYTE_DISTANCE) + ADDRESS_DISTANCE +
      BYTE_PER_LINE * (HI_LO_DISTANCE + BYTE_DISTANCE) + BYTE_DISTANCE;
  u32 max_y = y + ((size / BYTE_PER_LINE) + 1) * ROW_DISTANCE;

  assert(max_x < WIDTH && "x out of range of screen");
  assert(max_y < HEIGHT && "y out of range of screen");

  //write 16 bytes in each line of code in each line
  for(int height = 0; height <= (size / BYTE_PER_LINE); height++)
  {
    //print address of line
    for(int i = (X_LEN / 8) - 1; i >= 0; i--)
    {
      drawPattern(x, y, number_patterns_.at((address >> (i * 8 + 4)) & 0xF), NONE);
      x += HI_LO_DISTANCE;
      drawPattern(x, y, number_patterns_.at((address >> i * 8) & 0xF), NONE);
      x += BYTE_DISTANCE;
    }
    x += ADDRESS_DISTANCE;

    //print every byte for this line, highlight it if it is part of the currently executed instruction
    for(int width = 0; width < BYTE_PER_LINE; width++)
    {
      Highlight highlight = getHighlight(address);

      u8 byte = cpu_.read(address);
      drawPattern(x, y, number_patterns_.at((byte >> 4) & 0xF), highlight);
      x += HI_LO_DISTANCE;
      drawPattern(x, y, number_patterns_.at(byte & 0xF), highlight);
      x += BYTE_DISTANCE;

      if(width == ((BYTE_PER_LINE / 2) - 1))
      {
        x += BYTE_DISTANCE;
      }

      if(++address == end_address)
      {
        break;
      }
    }

    y += ROW_DISTANCE;
    x = 1;
  }
}

void Emulator::drawRegisters()
{
  int x = 1 + (X_LEN / 8) * (HI_LO_DISTANCE + BYTE_DISTANCE) + ADDRESS_DISTANCE +
      BYTE_PER_LINE * (HI_LO_DISTANCE + BYTE_DISTANCE) + 3 * BYTE_DISTANCE;
  int y = 1;

  int number = 0;

  for(auto &pattern : register_name_patterns_)
  {
    drawPattern(x, y, pattern, NONE);

    x += ADDRESS_DISTANCE;

    for(int i = 0; i < NUMBER_WIDTH; i++)
    {
      field_.at(y + 2).at(x + i) = WHITE;
      field_.at(y + 4).at(x + i) = WHITE;
    }

    x += BYTE_DISTANCE;

    Highlight highlight = (saved_registers_[number] != cpu_.getRegister(number)) ? CHANGED : NONE;

    for(int i = 3; i >= 0; i--)
    {
      drawPattern(x, y, number_patterns_.at((cpu_.getRegister(number) >> (i * 8 + 4)) & 0xF), highlight);
      x += HI_LO_DISTANCE;
      drawPattern(x, y, number_patterns_.at((cpu_.getRegister(number) >> i * 8) & 0xF), highlight);
      x += BYTE_DISTANCE;
    }

    y += ROW_DISTANCE;
    x = 1 + (X_LEN / 8) * (HI_LO_DISTANCE + BYTE_DISTANCE) + ADDRESS_DISTANCE +
      BYTE_PER_LINE * (HI_LO_DISTANCE + BYTE_DISTANCE) + 3 * BYTE_DISTANCE;

    number++;
  }
}

void Emulator::initRegisterNamePatterns()
{
  register_name_patterns_ = {
    //x0
    {
      {2, 0}, {2, 4}, {3, 1}, {3, 3}, {4, 2}, {5, 1}, {5, 3}, {6, 0},
      {6, 4},
      {0, 7}, {0, 8}, {0, 9}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6},
      {6, 7}, {6, 8}, {6, 9}, {5, 10}, {4, 10}, {3, 10}, {2, 10}, {1, 10}
    },
    //ra
    {
      {2, 1}, {2, 2}, {2, 3}, {3, 0}, {3, 4}, {4, 0}, {4, 4}, {5, 0},
      {6, 0}, {2, 6},
      {1, 7}, {1, 8}, {1, 9}, {2, 10}, {3, 10}, {4, 6}, {4, 7}, {4, 8},
      {4, 9}, {4, 10}, {5, 6}, {5, 10}, {6, 6}, {6, 7}, {6, 8}, {6, 9},
      {6, 10}
    },
    //sp
    {
      {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 0}, {4, 0}, {4, 1},
      {4, 2}, {4, 3}, {4, 4}, {5, 4}, {6, 4}, {6, 3}, {6, 2}, {6, 1},
      {6, 0},
      {2, 6}, {2, 7}, {2, 8}, {2, 9}, {3, 6}, {3, 10}, {4, 6}, {4, 7},
      {4, 8}, {4, 9}, {5, 6}, {6, 6}
    },
    //gp
    {
      {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 0}, {3, 4}, {4, 1}, {4, 2},
      {4, 3}, {4, 4}, {5, 4}, {6, 0}, {6, 1}, {6, 2}, {6, 3}, {6, 4},
      {2, 6}, {2, 7}, {2, 8}, {2, 9}, {3, 6}, {3, 10}, {4, 6}, {4, 7},
      {4, 8}, {4, 9}, {5, 6}, {6, 6}
    },
    //tp
    {
      {0, 2}, {1, 2}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 2},
      {4, 2}, {5, 2}, {5, 4}, {6, 2}, {6, 3}, {6, 4},
      {2, 6}, {2, 7}, {2, 8}, {2, 9}, {3, 6}, {3, 10}, {4, 6}, {4, 7},
      {4, 8}, {4, 9}, {5, 6}, {6, 6}
    },
    //t0
    {
      {0, 2}, {1, 2}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 2},
      {4, 2}, {5, 2}, {5, 4}, {6, 2}, {6, 3}, {6, 4},
      {0, 7}, {0, 8}, {0, 9}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6},
      {6, 7}, {6, 8}, {6, 9}, {5, 10}, {4, 10}, {3, 10}, {2, 10}, {1, 10}
    },
    //t1
    {
      {0, 2}, {1, 2}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 2},
      {4, 2}, {5, 2}, {5, 4}, {6, 2}, {6, 3}, {6, 4},
      {0, 7}, {0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 8},
      {6, 6}, {6, 7}, {6, 9}, {6, 10}
    },
    //t2
    {
      {0, 2}, {1, 2}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 2},
      {4, 2}, {5, 2}, {5, 4}, {6, 2}, {6, 3}, {6, 4},
      {1, 6}, {1, 10}, {0, 7}, {0, 8}, {0, 9}, {6, 6}, {6, 7}, {6, 8},
      {6, 9}, {6, 10}, {5, 6}, {5, 7}, {4, 7}, {4, 8}, {3, 8}, {3, 9},
      {2, 9}, {2, 10}
    },
    //fp
    {
      {0, 2}, {0, 3}, {0, 4}, {1, 2}, {1, 4}, {2, 2}, {3, 0}, {3, 1},
      {3, 2}, {3, 3}, {3, 4}, {4, 2}, {5, 2}, {6, 2},
      {2, 6}, {2, 7}, {2, 8}, {2, 9}, {3, 6}, {3, 10}, {4, 6}, {4, 7},
      {4, 8}, {4, 9}, {5, 6}, {6, 6}
    },
    //s1
    {
      {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 0}, {4, 0}, {4, 1},
      {4, 2}, {4, 3}, {4, 4}, {5, 4}, {6, 4}, {6, 3}, {6, 2}, {6, 1},
      {6, 0},
      {0, 7}, {0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 8},
      {6, 6}, {6, 7}, {6, 9}, {6, 10}
    },
    //a0
    {
      {1, 1}, {1, 2}, {1, 3}, {2, 4}, {3, 4}, {4, 0}, {4, 1}, {4, 2},
      {4, 3}, {4, 4}, {5, 0}, {5, 4}, {6, 0}, {6, 1}, {6, 2}, {6, 3},
      {6, 4},
      {0, 7}, {0, 8}, {0, 9}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6},
      {6, 7}, {6, 8}, {6, 9}, {5, 10}, {4, 10}, {3, 10}, {2, 10}, {1, 10}
    },
    //a1
    {
      {1, 1}, {1, 2}, {1, 3}, {2, 4}, {3, 4}, {4, 0}, {4, 1}, {4, 2},
      {4, 3}, {4, 4}, {5, 0}, {5, 4}, {6, 0}, {6, 1}, {6, 2}, {6, 3},
      {6, 4},
      {0, 7}, {0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 8},
      {6, 6}, {6, 7}, {6, 9}, {6, 10}
    },
    //a2
    {
      {1, 1}, {1, 2}, {1, 3}, {2, 4}, {3, 4}, {4, 0}, {4, 1}, {4, 2},
      {4, 3}, {4, 4}, {5, 0}, {5, 4}, {6, 0}, {6, 1}, {6, 2}, {6, 3},
      {6, 4},
      {1, 6}, {1, 10}, {0, 7}, {0, 8}, {0, 9}, {6, 6}, {6, 7}, {6, 8},
      {6, 9}, {6, 10}, {5, 6}, {5, 7}, {4, 7}, {4, 8}, {3, 8}, {3, 9},
      {2, 9}, {2, 10}
    },
    //a3
    {
      {1, 1}, {1, 2}, {1, 3}, {2, 4}, {3, 4}, {4, 0}, {4, 1}, {4, 2},
      {4, 3}, {4, 4}, {5, 0}, {5, 4}, {6, 0}, {6, 1}, {6, 2}, {6, 3},
      {6, 4},
      {1, 6}, {0, 7}, {0, 8}, {0, 9}, {1, 10}, {2, 10}, {3, 9}, {3, 8},
      {3, 7}, {5, 6}, {6, 7}, {6, 8}, {6, 9}, {5, 10}, {4, 10}
    },
    //a4
    {
      {1, 1}, {1, 2}, {1, 3}, {2, 4}, {3, 4}, {4, 0}, {4, 1}, {4, 2},
      {4, 3}, {4, 4}, {5, 0}, {5, 4}, {6, 0}, {6, 1}, {6, 2}, {6, 3},
      {6, 4},
      {0, 6}, {1, 6}, {2, 6}, {3, 6}, {3, 7}, {3, 8}, {3, 9}, {3, 10},
      {1, 9}, {2, 9}, {4, 9}, {5, 9}, {6, 9}
    },
    //a5
    {
      {1, 1}, {1, 2}, {1, 3}, {2, 4}, {3, 4}, {4, 0}, {4, 1}, {4, 2},
      {4, 3}, {4, 4}, {5, 0}, {5, 4}, {6, 0}, {6, 1}, {6, 2}, {6, 3},
      {6, 4},
      {0, 6}, {0, 7}, {0, 8}, {0, 9}, {0, 10}, {1, 6}, {2, 6}, {3, 6},
      {3, 7}, {3, 8}, {3, 9}, {3, 10}, {4, 10}, {5, 10}, {6, 10}, {6, 9},
      {6, 8}, {6, 7}, {6, 6}
    },
    //a6
    {
      {1, 1}, {1, 2}, {1, 3}, {2, 4}, {3, 4}, {4, 0}, {4, 1}, {4, 2},
      {4, 3}, {4, 4}, {5, 0}, {5, 4}, {6, 0}, {6, 1}, {6, 2}, {6, 3},
      {6, 4},
      {0, 7}, {0, 8}, {0, 9}, {1, 10}, {1, 6}, {2, 6}, {3, 6}, {4, 6},
      {5, 6}, {3, 7}, {3, 8}, {3, 9}, {4, 10}, {5, 10}, {6, 9}, {6, 8},
      {6, 7}
    },
    //a7
    {
      {1, 1}, {1, 2}, {1, 3}, {2, 4}, {3, 4}, {4, 0}, {4, 1}, {4, 2},
      {4, 3}, {4, 4}, {5, 0}, {5, 4}, {6, 0}, {6, 1}, {6, 2}, {6, 3},
      {6, 4},
      {0, 6}, {0, 7}, {0, 8}, {0, 9}, {0, 10}, {1, 10}, {2, 10}, {3, 9},
      {4, 8}, {5, 7}, {6, 6}
    },
    //s2
    {
      {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 0}, {4, 0}, {4, 1},
      {4, 2}, {4, 3}, {4, 4}, {5, 4}, {6, 4}, {6, 3}, {6, 2}, {6, 1},
      {6, 0},
      {1, 6}, {1, 10}, {0, 7}, {0, 8}, {0, 9}, {6, 6}, {6, 7}, {6, 8},
      {6, 9}, {6, 10}, {5, 6}, {5, 7}, {4, 7}, {4, 8}, {3, 8}, {3, 9},
      {2, 9}, {2, 10}
    },
    //s3
    {
      {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 0}, {4, 0}, {4, 1},
      {4, 2}, {4, 3}, {4, 4}, {5, 4}, {6, 4}, {6, 3}, {6, 2}, {6, 1},
      {6, 0},
      {1, 6}, {0, 7}, {0, 8}, {0, 9}, {1, 10}, {2, 10}, {3, 9}, {3, 8},
      {3, 7}, {5, 6}, {6, 7}, {6, 8}, {6, 9}, {5, 10}, {4, 10}
    },
    //s4
    {
      {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 0}, {4, 0}, {4, 1},
      {4, 2}, {4, 3}, {4, 4}, {5, 4}, {6, 4}, {6, 3}, {6, 2}, {6, 1},
      {6, 0},
      {0, 6}, {1, 6}, {2, 6}, {3, 6}, {3, 7}, {3, 8}, {3, 9}, {3, 10},
      {1, 9}, {2, 9}, {4, 9}, {5, 9}, {6, 9}
    },
    //s5
    {
      {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 0}, {4, 0}, {4, 1},
      {4, 2}, {4, 3}, {4, 4}, {5, 4}, {6, 4}, {6, 3}, {6, 2}, {6, 1},
      {6, 0},
      {0, 6}, {0, 7}, {0, 8}, {0, 9}, {0, 10}, {1, 6}, {2, 6}, {3, 6},
      {3, 7}, {3, 8}, {3, 9}, {3, 10}, {4, 10}, {5, 10}, {6, 10}, {6, 9},
      {6, 8}, {6, 7}, {6, 6}
    },
    //s6
    {
      {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 0}, {4, 0}, {4, 1},
      {4, 2}, {4, 3}, {4, 4}, {5, 4}, {6, 4}, {6, 3}, {6, 2}, {6, 1},
      {6, 0},
      {0, 7}, {0, 8}, {0, 9}, {1, 10}, {1, 6}, {2, 6}, {3, 6}, {4, 6},
      {5, 6}, {3, 7}, {3, 8}, {3, 9}, {4, 10}, {5, 10}, {6, 9}, {6, 8},
      {6, 7}
    },
    //s7
    {
      {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 0}, {4, 0}, {4, 1},
      {4, 2}, {4, 3}, {4, 4}, {5, 4}, {6, 4}, {6, 3}, {6, 2}, {6, 1},
      {6, 0},
      {0, 6}, {0, 7}, {0, 8}, {0, 9}, {0, 10}, {1, 10}, {2, 10}, {3, 9},
      {4, 8}, {5, 7}, {6, 6}
    },
    //s8
    {
      {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 0}, {4, 0}, {4, 1},
      {4, 2}, {4, 3}, {4, 4}, {5, 4}, {6, 4}, {6, 3}, {6, 2}, {6, 1},
      {6, 0},
      {0, 7}, {0, 8}, {0, 9}, {1, 6}, {2, 6}, {3, 7}, {3, 8}, {3, 9},
      {2, 10}, {1, 10}, {4, 6}, {5, 6}, {6, 7}, {6, 8}, {6, 9}, {5, 10},
      {4, 10}
    },
    //s9
    {
      {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 0}, {4, 0}, {4, 1},
      {4, 2}, {4, 3}, {4, 4}, {5, 4}, {6, 4}, {6, 3}, {6, 2}, {6, 1},
      {6, 0},
      {0, 7}, {0, 8}, {0, 9}, {1, 6}, {2, 6}, {3, 7}, {3, 8}, {3, 9},
      {2, 10}, {1, 10}, {3, 10}, {4, 10}, {5, 10}, {6, 9}, {6, 8}, {6, 7},
      {5, 6}
    },
    //s10
    {
      {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 0}, {4, 0}, {4, 1},
      {4, 2}, {4, 3}, {4, 4}, {5, 4}, {6, 4}, {6, 3}, {6, 2}, {6, 1},
      {6, 0},
      {0, 7}, {0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 8},
      {6, 6}, {6, 7}, {6, 9}, {6, 10},
      {0, 13}, {0, 14}, {0, 15}, {1, 12}, {2, 12}, {3, 12}, {4, 12}, {5, 12},
      {6, 13}, {6, 14}, {6, 15}, {5, 16}, {4, 16}, {3, 16}, {2, 16}, {1, 16}
    },
    //s11
    {
      {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 0}, {4, 0}, {4, 1},
      {4, 2}, {4, 3}, {4, 4}, {5, 4}, {6, 4}, {6, 3}, {6, 2}, {6, 1},
      {6, 0},
      {0, 7}, {0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}, {6, 8},
      {6, 6}, {6, 7}, {6, 9}, {6, 10},
      {0, 13}, {0, 14}, {1, 14}, {2, 14}, {3, 14}, {4, 14}, {5, 14}, {6, 14},
      {6, 12}, {6, 13}, {6, 15}, {6, 16},
    },
    //t3
    {
      {0, 2}, {1, 2}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 2},
      {4, 2}, {5, 2}, {5, 4}, {6, 2}, {6, 3}, {6, 4},
      {1, 6}, {0, 7}, {0, 8}, {0, 9}, {1, 10}, {2, 10}, {3, 9}, {3, 8},
      {3, 7}, {5, 6}, {6, 7}, {6, 8}, {6, 9}, {5, 10}, {4, 10}
    },
    //t4
    {
      {0, 2}, {1, 2}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 2},
      {4, 2}, {5, 2}, {5, 4}, {6, 2}, {6, 3}, {6, 4},
      {0, 6}, {1, 6}, {2, 6}, {3, 6}, {3, 7}, {3, 8}, {3, 9}, {3, 10},
      {1, 9}, {2, 9}, {4, 9}, {5, 9}, {6, 9}
    },
    //t5
    {
      {0, 2}, {1, 2}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 2},
      {4, 2}, {5, 2}, {5, 4}, {6, 2}, {6, 3}, {6, 4},
      {0, 6}, {0, 7}, {0, 8}, {0, 9}, {0, 10}, {1, 6}, {2, 6}, {3, 6},
      {3, 7}, {3, 8}, {3, 9}, {3, 10}, {4, 10}, {5, 10}, {6, 10}, {6, 9},
      {6, 8}, {6, 7}, {6, 6}
    },
    //t6
    {
      {0, 2}, {1, 2}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {3, 2},
      {4, 2}, {5, 2}, {5, 4}, {6, 2}, {6, 3}, {6, 4},
      {0, 7}, {0, 8}, {0, 9}, {1, 10}, {1, 6}, {2, 6}, {3, 6}, {4, 6},
      {5, 6}, {3, 7}, {3, 8}, {3, 9}, {4, 10}, {5, 10}, {6, 9}, {6, 8},
      {6, 7}
    }
  };
}

