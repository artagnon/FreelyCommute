#include <experimental/filesystem>

#include "rM.hpp"

namespace fc::rM
{
  int main(int argc, const char *argv[])
  {
    bytestream Stream("blank.rm");
    Parser{Stream};
    Stream.close();
    return EXIT_SUCCESS;
  }
} // namespace fc::rM
