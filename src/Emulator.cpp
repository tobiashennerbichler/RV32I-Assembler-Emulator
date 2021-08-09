#include "../include/Emulator.h"
#include <fstream>
#include <cassert>

Emulator::Emulator()
{
  std::string numbers_file_name = "../include/numbers.bmp";
  std::string register_file_name = "../include/register_names.bmp";

  initWindow();
  initField();
  initPatterns(16, 3, numbers_file_name, numbers_);
  initPatterns(4, X_LEN, register_file_name, register_names_);
  checkScreenBoundaries();
}

//TODO: make patterns better
//TODO: make better checks if screen to small for drawing
//TODO: some more general formula for calculating the placements of the memory sections/registers

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

  run();
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

void Emulator::initPatterns(int elements_per_row, int elements_per_col, std::string &file_name, std::vector<Pattern> &pattern)
{
  std::ifstream bmp_file = std::ifstream(file_name, std::ios::binary);

  if(!bmp_file.is_open())
  {
    printf("%s could not be opened\n", file_name.c_str());
    exit(-1);
  }

  for(int i = 0; i < elements_per_col * elements_per_row; i++)
  {
    std::vector<int> vector;
    pattern.push_back(vector);
  }

  //skip BMP header
  for(int i = 0; i < BMP_HEADER_SIZE; i++)
  {
    u8 byte;
    bmp_file.read((char *) &byte, sizeof(u8));
  }

  int pixel = 0;

  //each color is made up of 3 bytes
  //reading starts at the bottom of the BMP file, five pixel belong to each number per row
  //first 7 rows are the normal numbers, second 7 rows the pc numbers and the last 7 rows the changed numbers
  while(!bmp_file.eof())
  {
    u8 byte_1, byte_2, byte_3;
    bmp_file.read((char *) &byte_1, sizeof(u8));
    bmp_file.read((char *) &byte_2, sizeof(u8));
    bmp_file.read((char *) &byte_3, sizeof(u8));

    int color = byte_1 | (byte_2 << 8) | (byte_3 << 16);

    int row = pixel / (elements_per_row * NUMBER_WIDTH);
    int number = (pixel % (elements_per_row * NUMBER_WIDTH)) / NUMBER_WIDTH;
    number += elements_per_row * (row / NUMBER_HEIGHT);

    pattern.at(number).push_back(color);

    pixel++;

    if(pixel == elements_per_row * NUMBER_WIDTH * elements_per_col * NUMBER_HEIGHT)
    {
      break;
    }
  }
}

void Emulator::checkScreenBoundaries()
{
  int x = 1, y = 1;

  x += (X_LEN / 8) * (HI_LO_DISTANCE + BYTE_DISTANCE) + ADDRESS_DISTANCE +
    BYTE_PER_LINE * (HI_LO_DISTANCE + BYTE_DISTANCE) + BYTE_DISTANCE;
  x += 2 * ADDRESS_DISTANCE + BYTE_DISTANCE + (X_LEN / 8) * (HI_LO_DISTANCE + BYTE_DISTANCE);

  y += X_LEN * ROW_DISTANCE;

  if(x >= WIDTH || y >= HEIGHT)
  {
    printf("too large for screen: %d, %d\n", x, y);
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

void Emulator::drawPattern(int x, int y, Pattern &pattern)
{
  int x_pos = 0, y_pos = NUMBER_HEIGHT - 1;

  for(auto &p : pattern)
  {
    field_.at(y + y_pos).at(x + x_pos) = p;
    if(++x_pos == NUMBER_WIDTH)
    {
      x_pos = 0;
      y_pos--;
    }
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

  u32 start = cpu_.getPC() - PC_SECTION;
  u32 end = cpu_.getPC() + PC_SECTION;
  drawMemorySection(x, y, start, end);

  y += ((int) ((end - start) / BYTE_PER_LINE) + 2) * ROW_DISTANCE;
  start = cpu_.getRegister(2) - SP_SECTION;
  end = cpu_.getRegister(2) + SP_SECTION;
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
      u32 number = (address >> (i * 8 + 4)) & 0xF;
      drawPattern(x, y, numbers_.at(number));
      x += HI_LO_DISTANCE;
      number = (address >> i * 8) & 0xF;
      drawPattern(x, y, numbers_.at(number));
      x += BYTE_DISTANCE;
    }
    x += ADDRESS_DISTANCE;

    //print every byte for this line, highlight it if it is part of the currently executed instruction
    for(int width = 0; width < BYTE_PER_LINE; width++)
    {
      Highlight highlight = getHighlight(address);

      u8 byte = cpu_.read(address);
      u32 number = ((byte >> 4) & 0xF) + highlight * 16;
      drawPattern(x, y, numbers_.at(number));
      x += HI_LO_DISTANCE;
      number = (byte & 0xF) + highlight * 16;
      drawPattern(x, y, numbers_.at(number));
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

  int index = 0;

  for(int j = 0; j < X_LEN; j++)
  {
    for(int i = 0; i < 4; i++)
    {
      drawPattern(x, y, register_names_.at(index));
      x += HI_LO_DISTANCE;
      index++;
    }

    x += BYTE_DISTANCE;

    Highlight highlight = (saved_registers_[index / 4] != cpu_.getRegister(index / 4)) ? CHANGED : NONE;

    for(int i = 3; i >= 0; i--)
    {
      u32 number = (cpu_.getRegister(index / 4) >> (i * 8 + 4)) & 0xF + highlight * 16;
      drawPattern(x, y, numbers_.at(number));
      x += HI_LO_DISTANCE;
      number = (cpu_.getRegister(index / 4) >> i * 8) & 0xF + highlight * 16;
      drawPattern(x, y, numbers_.at(number));
      x += BYTE_DISTANCE;
    }

    y += ROW_DISTANCE;
    x = 1 + (X_LEN / 8) * (HI_LO_DISTANCE + BYTE_DISTANCE) + ADDRESS_DISTANCE +
      BYTE_PER_LINE * (HI_LO_DISTANCE + BYTE_DISTANCE) + 3 * BYTE_DISTANCE;
  }
}

