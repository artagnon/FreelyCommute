#include <utility>
#include <cassert>

#include "rM.hpp"
#include "Utility.hpp"

namespace fc::rM
{
  Parser::Parser(std::ifstream &S) : Stream(S), Root(std::move(MiniParser<Page>{Stream}))
  {
    auto &Layers = fillChildren<Page, Layer>(Root);
    utility::for_each(Layers, [](auto E) {
      auto &Lines = fillChildren<Layer, Line>(E);
      utility::for_each(Lines, [](auto E) { fillChildren<Line, Point>(E) });
    });
  }
} // namespace fc::rM
