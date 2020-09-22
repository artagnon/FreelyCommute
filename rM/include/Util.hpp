#pragma once

#include <utility>
#include <cstdlib>
#include <iostream>

namespace fc::rM::util
{
  using i32 = uint32_t;

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

  inline i32 to_le(char *b)
  {
    return (b[0] & 0xFF) | ((b[1] << 8) & 0xFF) | ((b[2] << 16) & 0xFF) | ((b[3] << 24) & 0xFF);
  }

  template <typename T, typename U>
  constexpr bool decay_equiv_v = static_cast<bool>(std::is_same_v<std::decay_t<T>, U>);
} // namespace fc::rM::util
