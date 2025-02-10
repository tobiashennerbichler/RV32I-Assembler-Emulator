#include "../include/Emulator.h"
#include <fstream>
#include <cassert>

Emulator::Emulator()
{
  std::string ascii_file_name = "../include/ascii.bmp";

  initWindow();
  initField();
  initPatterns(128, 16, ascii_file_name, ascii_patterns_);
  initRegisterNames();
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

/*
 * draw memory section around pc, around sp and draw registers to the screen
 * calculate where the positions are, depending on the defines
 */
void Emulator::updateScreen()
{
  resetScreen();

  drawString(START_X, START_Y, "Code:");

  u32 start = cpu_.getPC() - PC_SECTION;
  u32 end = cpu_.getPC() + PC_SECTION;
  drawMemorySection(START_X, START_Y + ROW_DISTANCE, start, end);

  drawString(START_X, SP_Y, "Stack:");

  start = cpu_.getRegister(2) - SP_SECTION;
  end = cpu_.getRegister(2) + SP_SECTION;
  drawMemorySection(START_X, SP_Y + ROW_DISTANCE, start, end);

  drawString(REGISTER_X, START_Y, "Registers:");

  drawRegisters(REGISTER_X, START_Y + ROW_DISTANCE);
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

//important: pixels in each row of the bmp file have to be a multiple of 4 otherwise 0's are added
void Emulator::initPatterns(int elements, int elements_per_row, std::string &file_name, std::vector<Pattern> &pattern)
{
  std::ifstream bmp_file = std::ifstream(file_name, std::ios::binary);

  if(!bmp_file.is_open())
  {
    printf("%s could not be opened\n", file_name.c_str());
    exit(-1);
  }

  for(int i = 0; i < elements; i++)
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
  //reading starts at the bottom of the left of the picture, reads line per line from left to right
  //each NUMBER_WIDTH pixels the "target" changes, there are elements_per_row targets per row
  //each target is NUMBER_HEIGHT high and thus we have different targets every NUMBER_HEIGHT rows, there are elements_per_col different lines with targets
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

    if(pixel == elements * NUMBER_WIDTH * NUMBER_HEIGHT)
    {
      break;
    }
  }
}

void Emulator::initRegisterNames()
{
  register_names_ = {
      "x0",
      "ra",
      "sp",
      "gp",
      "tp",
      "t0",
      "t1",
      "t2",
      "fp",
      "s1",
      "a0",
      "a1",
      "a2",
      "a3",
      "a4",
      "a5",
      "a6",
      "a7",
      "s2",
      "s3",
      "s4",
      "s5",
      "s6",
      "s7",
      "s8",
      "s9",
      "s10",
      "s11",
      "t3",
      "t4",
      "t5",
      "t6"
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

void Emulator::drawPattern(int x, int y, Pattern &pattern, Highlight highlight)
{
  int x_pos = 0, y_pos = NUMBER_HEIGHT - 1;

  for(auto &p : pattern)
  {
    if(p == WHITE)
    {
      field_.at(y + y_pos).at(x + x_pos) = highlight;
    }

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

  //TODO: simplify this with a define, make a define for every start x and y
  u32 max_x = x + (X_LEN / 8) * (HI_LO_DISTANCE + BYTE_DISTANCE) + ADDRESS_DISTANCE +
      BYTE_PER_LINE * (HI_LO_DISTANCE + BYTE_DISTANCE) + BYTE_DISTANCE;
  u32 max_y = y + ((size / BYTE_PER_LINE) + 1) * ROW_DISTANCE;

  assert(max_x <= WIDTH && "x out of range of screen");
  assert(max_y <= HEIGHT && "y out of range of screen");

  //write BYTE_PER_LINE bytes of memory per line
  for(int height = 0; height <= (size / BYTE_PER_LINE); height++)
  {
    //first print address of line
    for(int i = (X_LEN / 8) - 1; i >= 0; i--)
    {
      //first print higher four bits
      u32 number = getAsciiNumber((address >> (i * 8 + 4)) & 0xF);
      drawPattern(x, y, ascii_patterns_.at(number), NONE);
      x += HI_LO_DISTANCE;

      //then print lower four bits
      number = getAsciiNumber((address >> i * 8) & 0xF);
      drawPattern(x, y, ascii_patterns_.at(number), NONE);
      x += BYTE_DISTANCE;
    }
    x += ADDRESS_DISTANCE;

    //print every byte for this line, highlight it if it is part of the current executed instruction
    for(int width = 0; width < BYTE_PER_LINE; width++)
    {
      Highlight highlight = getHighlight(address);

      //first print higher four bits
      u8 byte = cpu_.read(address);
      u32 number = getAsciiNumber((byte >> 4) & 0xF);
      drawPattern(x, y, ascii_patterns_.at(number), highlight);
      x += HI_LO_DISTANCE;

      //and then lower four bits
      number = getAsciiNumber(byte & 0xF);
      drawPattern(x, y, ascii_patterns_.at(number), highlight);
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
    x = START_X;
  }
}

void Emulator::drawRegisters(int x, int y)
{
  for(int j = 0; j < X_LEN; j++)
  {
    drawString(x, y, register_names_.at(j));
    x = REGISTER_X + 4 * HI_LO_DISTANCE;

    drawString(x, y, "=");
    x += BYTE_DISTANCE;

    Highlight highlight = (saved_registers_[j] != cpu_.getRegister(j)) ? CHANGED : NONE;

    for(int i = ((X_LEN / 8) - 1); i >= 0; i--)
    {
      u32 number = getAsciiNumber((cpu_.getRegister(j) >> (i * 8 + 4)) & 0xF);
      drawPattern(x, y, ascii_patterns_.at(number), highlight);
      x += HI_LO_DISTANCE;
      number = getAsciiNumber((cpu_.getRegister(j) >> i * 8) & 0xF);
      drawPattern(x, y, ascii_patterns_.at(number), highlight);
      x += BYTE_DISTANCE;
    }

    y += ROW_DISTANCE;
    x = REGISTER_X;
  }
}

void Emulator::drawString(int x, int y, std::string string)
{
  assert((x + string.size() * HI_LO_DISTANCE) <= WIDTH && "x out of range");

  for(auto &s : string)
  {
    drawPattern(x, y, ascii_patterns_.at(s), NONE);
    x += HI_LO_DISTANCE;
  }
}

u32 Emulator::getAsciiNumber(u32 number)
{
  assert(number >= 0 && number < 16 && "number > 16 or < 0");

  if(number > 9)
  {
    return ((number % 10) + 'A');
  }

  return (number + '0');
}

