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

  // Get the header out of the way
  constexpr char ExpectedHdr[] = "reMarkable .lines file, version=5          ";
  constexpr size_t Sz = sizeof(ExpectedHdr);
  char ActualHdr[Sz - 1]; // excluding nul terminator
  Stream.read(static_cast<char *>(ActualHdr), Sz - 1);
  util::assert(!strncmp(ActualHdr, ExpectedHdr, Sz - 1), "Header mismatched; please use a valid .rM v5 file");

  std::cout << Parser{Stream} << std::endl;
  util::assert(Stream.get() == -1, "End of file marker was not found");
  util::assert(Stream.eof(), "Some bytes were found at the end of the stream");
  Stream.close();
  return EXIT_SUCCESS;
}
