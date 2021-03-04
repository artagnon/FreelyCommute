#include <iostream>

#include "rM/Parser.hpp"
#include "rM/Printer.hpp"

using namespace fc::rM;

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " <.rM file>" << std::endl;
    return EXIT_FAILURE;
  }
  std::ifstream Stream(argv[1], std::ios::binary);
  util::assert_msg(Stream.is_open(), "Unable to open input file");
  util::check_rM_hdr(Stream);
  std::cout << Parser{Stream} << std::endl;
  util::check_eof(Stream);
  Stream.close();
  return EXIT_SUCCESS;
}
