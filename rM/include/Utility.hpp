#pragma once

#include <utility>

namespace fc::rM::utility
{
  template <typename L>
  void repeat(int N, L Lambda)
  {
    for (auto i = 0; i < N; ++i)
      static_cast<void>(Lambda);
  }
  template <typename I, typename L>
  constexpr void for_each(I Iterable, L Lambda) { std::for_each(Iterable.begin(), Iterable.end(), Lambda); }
} // namespace fc::rM::utility
