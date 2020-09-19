#include <experimental/filesystem>

#include "rM.hpp"

namespace fc::rM
{
  int main(int argc, const char *argv[])
  {
    std::ifstream Stream("blank.rm", std::ios::binary);
    Parser{Stream};
    Stream.close();
    return EXIT_SUCCESS;
  }
} // namespace fc::rM
