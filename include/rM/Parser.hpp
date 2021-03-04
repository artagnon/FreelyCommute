#pragma once

#include <fstream>
#include <variant>

#include "ParseTable.hpp"

namespace fc::rM
{
  class Parser
  {
    std::ifstream &Stream;
    Page Root;

    template <typename T>
    T miniParser() const
    {
      T Record;
      for (auto &&elt : tablemap::M<T>)
      {
        i32 Scratch = 0;
        Stream.read(reinterpret_cast<char *>(&Scratch), sizeof(Scratch));
        elt.assign(Record, Scratch);
      }
      return Record;
    };

    template <size_t i, typename... Ts, typename CurTy>
    void recurseFillChildren(CurTy &E)
    {
      using PackTy = std::variant<Ts...>;
      using TyL = std::variant_alternative_t<i - 1, PackTy>;
      static_assert(std::is_same_v<CurTy, TyL>);
      using TyR = std::variant_alternative_t<i, PackTy>;

      for (i32 j = 0; j < E.NChildren; ++j)
      {
        E.Children.push_back(miniParser<TyR>());
        if constexpr (i + 1 < sizeof...(Ts))
          recurseFillChildren<i + 1, Ts...>(E.Children.back());
      }
    };

  public:
    inline Parser(std::ifstream &S) : Stream(S), Root(miniParser<Page>())
    {
      recurseFillChildren<1, Page, Layer, Line, Point>(Root);
    }
    inline operator Page() const { return Root; }
  };
} // namespace fc::rM
