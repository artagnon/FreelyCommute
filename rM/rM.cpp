#include <utility>
#include <cassert>
#include <variant>

#include "rM.hpp"
#include "Utility.hpp"

namespace fc::rM
{
  namespace tablemap
  {
    template <>
    auto M<Page> = std::make_pair(PageTable, 15);
    template <>
    auto M<Layer> = std::make_pair(LayerTable, 1);
    template <>
    auto M<Line> = std::make_pair(LineTable, 14);
    template <>
    auto M<Point> = std::make_pair(PointTable, 3);
  } // namespace tablemap

  Parser::Parser(std::ifstream &S) : Stream(S), Root(std::move(MiniParser<Page>{Stream}))
  {
    fillChildren<Page, Layer>(Root);
    utility::for_each(Root.Children, [&](auto E) {
      fillChildren<Layer, Line>(E);
      utility::for_each(E.Children, [&](auto E) { fillChildren<Line, Point>(E); });
    });
  }
} // namespace fc::rM
