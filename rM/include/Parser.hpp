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
      for (auto &&elt : tablemap::M<T>)
      {
        Stream.ignore(elt.toSkip());
        if (elt.toSkip())
          continue;
        elt.assign(Record, Stream.get());
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
    inline Parser(std::ifstream &S) : Stream(S), Root(miniParser<Page>())
    {
      recurseFillChildren<1, Page, Layer, Line, Point>(Root);
    }
    inline operator Page() const { return Root; }
  };
} // namespace fc::rM
