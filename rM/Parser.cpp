#include "Parser.hpp"
#include "Utility.hpp"

namespace fc::rM
{
  Parser::Parser(std::ifstream &S) : Stream(S), Root(std::move(MiniParser<Page>{Stream}))
  {
    fillChildren<Page, Layer>(Root);
    utility::for_each(Root.Children, [&](auto E) {
      fillChildren<Layer, Line>(E);
      utility::for_each(E.Children, [&](auto E) { fillChildren<Line, Point>(E); });
    });
  }
} // namespace fc::rM
