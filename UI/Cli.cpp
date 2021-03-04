#include <iostream>

#include "rM/Structures.hpp"
#include "rM/Parser.hpp"
#include "SVG/Writer.hpp"

using namespace fc::rM;
using namespace fc::svg;

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    std::cout << "Usage: " << argv[0] << " <.rM file> <.svg file>" << std::endl;
    return EXIT_FAILURE;
  }
  std::ifstream Stream(argv[1], std::ios::binary);
  util::assert_msg(Stream.is_open(), "Unable to open input file");
  util::check_rM_hdr(Stream);

  Page P = Parser{Stream};

  Dimensions Dim(1404, 1872);
  Document Doc(argv[2], Layout(Dim, Layout::TopLeft));

  for (auto &Layer : P)
  {
    for (auto &Line : Layer)
    {
      Polyline L(Stroke(0.5, Color::Black));
      for (auto &P : Line)
      {
        L << fc::svg::Point(P.X, P.Y);
      }
      Doc << L;
    }
  }

  Doc.save();

  util::check_eof(Stream);
  Stream.close();
  return EXIT_SUCCESS;
}
