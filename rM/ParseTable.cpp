#include <utility>

#include "ParseTable.hpp"

namespace fc::rM::tablemap
{
  template <>
  auto M<Page> = std::make_pair(PageTable, 31);
  template <>
  auto M<Layer> = std::make_pair(LayerTable, 1);
  template <>
  auto M<Line> = std::make_pair(LineTable, 14);
  template <>
  auto M<Point> = std::make_pair(PointTable, 3);
} // namespace fc::rM::tablemap
