#include <string>

class TestSystem
{
  public:
    static bool testFiles();

  private:
    static bool testFile(std::string &file_name, std::string &ref_file_name);
};