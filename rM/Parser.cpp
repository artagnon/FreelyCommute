#include "Parser.hpp"
#include "Util.hpp"

namespace fc::rM
{
  Parser::Parser(std::ifstream &S) : Stream(S), Root(std::move(MiniParser<Page>{Stream}))
  {
    fillChildren<Page, Layer>(Root);
    util::for_each(Root.Children, [&](auto E) {
      fillChildren<Layer, Line>(E);
      util::for_each(E.Children, [&](auto E) { fillChildren<Line, Point>(E); });
    });
  }
} // namespace fc::rM
