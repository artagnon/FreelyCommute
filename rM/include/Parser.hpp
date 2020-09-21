#pragma once

#include <fstream>

#include "ParseTable.hpp"
#include "Structures.hpp"

namespace fc::rM
{
  class Parser
  {
    std::ifstream &Stream;
    Page Root;

    template <typename T>
    T miniParser()
    {
      T Record;
      auto [M, Sz] = tablemap::M<T>;
      for (size_t i = 0; i < Sz; ++i)
      {
        i64 S = 0;
        Stream.read(reinterpret_cast<char *>(&S), sizeof(S));
        M[i].assertOrAssign(Record, S);
      }
      return Record;
    };

    template <typename TyL, typename TyR>
    void fillChildren(TyL &E)
    {
      for (i32 i = 0; i < E.NChildren; ++i)
      {
        E.Children.push_back(miniParser<TyR>());
      }
    };

    template <size_t i, typename... Ts, typename CurTy>
    void recurseFillChildren(CurTy &E)
    {
      using PackTy = std::variant<Ts...>;
      static_assert(std::is_same_v<CurTy, std::variant_alternative_t<i - 1, PackTy>>);
      using TyL = std::variant_alternative_t<i - 1, PackTy>;
      using TyR = std::variant_alternative_t<i, PackTy>;

      fillChildren<TyL, TyR>(E);
      if constexpr (i + 1 < sizeof...(Ts))
        std::for_each(E.Children.begin(), E.Children.end(), [this](auto &E) { recurseFillChildren<i + 1, Ts...>(E); });
    };

  public:
    inline Parser(std::ifstream &S) : Stream(S), Root(std::move(miniParser<Page>()))
    {
      recurseFillChildren<1, Page, Layer, Line, Point>(Root);
    }
    inline operator Page() const { return Root; }
  };
} // namespace fc::rM
