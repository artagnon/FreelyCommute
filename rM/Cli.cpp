#include <experimental/filesystem>
#include <iostream>

#include "Parser.hpp"
#include "Printer.hpp"

using namespace fc::rM;

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " <.rM file>" << std::endl;
    return EXIT_FAILURE;
  }
  std::ifstream Stream(argv[1], std::ios::binary);
  util::assert(Stream.is_open(), "Unable to open input file");
  std::cout << Parser{Stream} << std::endl;
  util::assert(Stream.eof(), "Some bytes were found at the end of the stream");
  Stream.close();
  return EXIT_SUCCESS;
}
