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
        util::assert(!Stream.eof(), "End of stream reached prematurely");
        auto Field = Record.*(elt.Ptr);
        Stream.read(reinterpret_cast<char *>(&Field), sizeof(Field));
        util::to_le(Field, reinterpret_cast<char *>(&Field));
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
        E.Children[j] = std::move(miniParser<TyR>());
        if constexpr (i + 1 < sizeof...(Ts))
          recurseFillChildren<i + 1, Ts...>(E.Children[j]);
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
