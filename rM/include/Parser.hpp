#pragma once

#include <fstream>

#include "ParseTable.hpp"
#include "Structures.hpp"

namespace fc::rM
{
  template <typename T>
  struct MiniParser
  {
    std::ifstream &Stream;
    MiniParser(std::ifstream &S) : Stream(S) {}
    T Record;
    operator T()
    {
      auto [M, Sz] = tablemap::M<T>;
      for (auto i = 0; i < Sz; ++i)
      {
        i64 S = 0;
        Stream.read(reinterpret_cast<char *>(&S), sizeof(S));
        M[i].assertOrAssign(Record, S);
      }
      return Record;
    }
  };

  class Parser
  {
    std::ifstream &Stream;
    Page Root;

    template <typename ParentTy, typename ChildTy>
    void fillChildren(ParentTy &B)
    {
      for (i32 i = 0; i < B.NChildren; ++i)
      {
        B.Children.push_back(MiniParser<ChildTy>{Stream});
      }
    }

  public:
    Parser(std::ifstream &S);
    inline operator Page() const { return Root; }
  };
} // namespace fc::rM
