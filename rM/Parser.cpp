#include <algorithm>

#include "Parser.hpp"

namespace fc::rM
{
  Parser::Parser(std::ifstream &S) : Stream(S), Root(std::move(MiniParser<Page>{Stream}))
  {
    fillChildren<Page, Layer>(Root);
    std::for_each(Root.Children.begin(), Root.Children.end(), [this](auto &E) {
      fillChildren<Layer, Line>(E);
      std::for_each(E.Children.begin(), E.Children.end(), [this](auto &E) { fillChildren<Line, Point>(E); });
    });
  }
} // namespace fc::rM
