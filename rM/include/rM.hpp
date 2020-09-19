#pragma once

#include <utility>
#include <new>
#include <fstream>
#include <variant>
#include <vector>

#include "Utility.hpp"

/* Format spec:

00000000: 7265 4d61 726b 6162 6c65 202e 6c69 6e65  reMarkable .line
00000010: 7320 6669 6c65 2c20 7665 7273 696f 6e3d  s file, version=
===================================================================
                   file type information

00000020: 3520 2020 2020 2020 2020 2001 0000 0000  5          .....
          ==== ~~~~~~~~~~~~~~~~~~~~~~^^ nlayers (i32)
                        padding
00000030: 0000 00                                  ...

00000020: 3520 2020 2020 2020 2020 2001 0000 0001  5          .....
                                        ~~~~ ~~^^ nlines (i132)
                                        seek 3
         |-> first line header follows
00000030: 0000 0011 0000 0000 0000 0000 0000 0000  ................
          ~~~~ ~~^^ brush type (i32)
           seek 3

00000030: 0000 000d 0000 0000 0000 0000 0000 0000  ................
                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                           color?
00000040: 0000 4000 0000 001d 0000 008d 8c3d 434b  ..@..........=CK
          ~~~~~~~~~~~~~~ ~~
            skip 18

00000040: 0000 4000 0000 0018 0000 00c6 0002 4342  ..@...........CB
                           ^^
                   brush base size (f32)

00000340: 0000 4000 0000 001b 0000 00c8 b099 43b8  ..@...........C.
                              ~~~~ ~~
                               skip 3

00000340: 0000 4000 0000 001b 0000 00c8 b099 43b8  ..@...........C.
                                     ^^ |-> first point header follows
                              npoints (i32)
*/

namespace fc::rM
{
  using i32 = uint8_t;
  using i64 = uint16_t;

  template <typename F>
  struct AttachChildren
  {
    i32 NChildren;
    std::vector<F> Children;
  };

  struct Point : public AttachChildren<std::nullptr_t>
  {
    i32 X;
    i32 Y;
    i32 Speed;
    i32 Direction;
    i32 Width;
    i32 Pressure;
  };

  struct Line : public AttachChildren<Point>
  {
    i32 BrushColor;
    i32 BrushType;
    i32 BrushBaseSize;
  };

  using Layer = AttachChildren<Line>;
  using Page = AttachChildren<Layer>;

  template <typename Record>
  class TableEntry
  {
    using F = i32 Record::*; // Field accessor
    using PE = std::variant<i32, F>;
    std::variant<i64, std::pair<PE, PE>> Raw;

  public:
    constexpr TableEntry(int V) : Raw(static_cast<i64>(V)) {}
    constexpr TableEntry(int Fst, F Snd) : Raw(std::make_pair(static_cast<i32>(Fst), Snd)) {}
    constexpr TableEntry(F Fst, F Snd) : Raw(std::make_pair(Fst, Snd)) {}
    constexpr TableEntry(std::nullptr_t) {}

    template <typename PtrOrSz, typename Sz>
    void assertOrAssign(Record R, PtrOrSz &FieldOrConstant, Sz Bytes)
    {
      if (std::is_same_v<PtrOrSz, Sz>)
      {
        assert(FieldOrConstant == Bytes);
        return;
      }
      static_assert(std::is_same_v<PtrOrSz, F>);
      auto FieldPtr = FieldOrConstant;
      R.*Field = Bytes;
    }

    void assertOrAssign(Record R, i64 DByte)
    {
      if (std::holds_alternative<std::pair<PE, PE>>(Raw))
      {
        auto &[RawFst, RawSnd] = Raw;
        assertOrAssign(R, RawFst, DByte % 0x100);
        assertOrAssign(R, RawSnd, DByte / 0x100);
        return;
      }
      assertOrAssign(R, Raw, DByte);
    }
  };

  constexpr std::nullptr_t Ghost = nullptr;

  // Unfortunately std::array is not mature enough to be used for nested aggregate initialization

  constexpr TableEntry<i32 Page::*> PageTable[15] = {
      0x3520, 0x2020, 0x2020, 0x2020, 0x2020, 0x2001, 0x0000, 0x0000, 0x3520, 0x2020, 0x2020, 0x2020, 0x2020, {0x20, &Page::NChildren}, 0x0000};

  constexpr TableEntry<i32 Layer::*> LayerTable[1] = {{0x00, &Layer::NChildren}};

  constexpr TableEntry<i32 Line::*> LineTable[14] = {Ghost, {0x00, &Line::BrushType}, Ghost, Ghost, Ghost, Ghost, Ghost, Ghost, Ghost, Ghost, Ghost, Ghost, Ghost, {0x00, &Line::NChildren}};

  constexpr TableEntry<i32 Point::*> PointTable[4] = {
      {&Point::X, &Point::Y}, {&Point::Speed, &Point::Direction}, {&Point::Width, &Point::Pressure}};

  namespace tablemap
  {
    template <typename F>
    std::pair<TableEntry<i32 F::*> *, size_t> M;
    template <>
    auto M<Page> = PageTable;
    template <>
    auto M<Layer> = LayerTable;
    template <>
    auto M<Line> = LineTable;
    template <>
    auto M<Point> = PointTable;
  } // namespace tablemap

  template <typename F>
  struct MiniParser
  {
    std::ifstream &Stream;
    MiniParser(std::ifstream &S) : Stream(S) {}
    F Record;
    operator F()
    {
      auto [M, Sz] = tablemap::M<F>;
      for (int i = 0; i < Sz; ++i)
      {
        uint16_t S;
        Stream.read(reinterpret_cast<char *>(S), sizeof(S));
        M[i].assertOrAssign(Record, S);
      }
      return Record;
    }
  };

  class Parser
  {
    Page Root;
    std::ifstream &Stream;

    template <typename ParentTy, typename ChildTy>
    std::vector<ChildTy> fillChildren(ParentTy &B)
    {
      utility::repeat(B.NChildren, [&]() { B.Children.emplace_back(MiniParser<ChildTy>{Stream}); });
      return B.Children;
    }

  public:
    Parser(std::ifstream &S);
  };
} // namespace fc::rM
