#include "test/ut.hpp"

#include "rM/Structures.hpp"
#include "rM/Parser.hpp"

using namespace boost::ut;
using namespace fc::rM;

int main()
{
  "header"_test = [] {
    std::ifstream Stream("blank.rm", std::ios::binary);
    expect(Stream.is_open()) << "Unable to open stream";
    constexpr char ExpectedHdr[] = "reMarkable .lines file, version=5          ";
    constexpr size_t Sz = sizeof(ExpectedHdr);
    char ActualHdr[Sz - 1]; // excluding nul terminator
    Stream.read(static_cast<char *>(ActualHdr), Sz - 1);
    expect(!strncmp(ActualHdr, ExpectedHdr, Sz - 1)) << "Header mismatched";
    Stream.close();
  };

  "pen-line"_test = [] {
    std::ifstream Stream("pen-line.rm", std::ios::binary);
    expect(Stream.is_open()) << "Unable to open stream";
    util::check_rM_hdr(Stream);
    Page P = Parser{Stream};
    expect(P.NChildren == 1) << "Page has more than one layer in it";
    for (auto &Layer : P)
    {
      expect(Layer.NChildren == 1) << "Page has more than one line in it";
    }
    util::check_eof(Stream);
    Stream.close();
  };
}
