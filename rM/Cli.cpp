#include <experimental/filesystem>
#include <iostream>

#include "Parser.hpp"

using namespace fc::rM;

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: %s <.rM file>" << argv[0];
    return EXIT_FAILURE;
  }
  std::ifstream Stream(argv[1], std::ios::binary);
  Parser{Stream};
  Stream.close();
  return EXIT_SUCCESS;
}
