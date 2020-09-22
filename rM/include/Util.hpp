#pragma once

#include <utility>
#include <cstdlib>
#include <iostream>

namespace fc::rM::util
{
  using i32 = uint8_t;
  using i64 = uint16_t;

  template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
  constexpr void assert_eq(T Expected, T Actual)
  {
    if (Actual != Expected)
    {
      std::cout << "Expected: " << std::hex << std::showbase << +Expected << std::endl;
      std::cout << "Actual: " << std::hex << std::showbase << +Actual << std::endl;
      std::abort();
    }
  }

  constexpr void assert(bool Condition, const char Msg[])
  {
    if (!Condition)
    {
      std::cout << Msg << std::endl;
      std::abort();
    }
  }

  // little endian helpers
  constexpr i64 byteswap(i64 Read)
  {
    return (Read >> 8) | (Read << 8);
  }

  constexpr std::pair<i32, i32> byteswap_pair(i64 Read)
  {
    return std::make_pair(Read, Read >> 8);
  }
} // namespace fc::rM::util
