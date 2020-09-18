#include <utility>
#include <cassert>

#include "rM.hpp"

namespace fc::rM
{
  Parser::Parser(bytestream &Stream)
  {
    i32 Scratch[2];
    for (auto &&Entry : SigTable)
    {
      std::visit([](auto x) {}, Entry);
    }
  }
} // namespace fc::rM
