#include <experimental/filesystem>

#include "rM.hpp"

using namespace fc::rM;

int main()
{
  std::ifstream Stream("blank.rm", std::ios::binary);
  Parser{Stream};
  Stream.close();
  return EXIT_SUCCESS;
}
