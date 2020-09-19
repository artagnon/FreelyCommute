#include <utility>
#include <cassert>

#include "rM.hpp"
#include "Utility.hpp"

namespace fc::rM
{
  Parser::Parser(std::ifstream &Stream)
  {
    utility::for_each(PageTable, [](auto E) {});
    utility::for_each(LayerTable, [](auto E) {});
    utility::for_each(LineTable, [](auto E) {});
    utility::for_each(PointTable, [](auto E) {});
  }
} // namespace fc::rM
