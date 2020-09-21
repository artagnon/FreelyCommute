#pragma once

#include <new>
#include <fstream>
#include <variant>

#include "Util.hpp"
#include "Structures.hpp"

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
  template <typename FieldPtr>
  class TableEntry
  {
    using PE = std::variant<i32, FieldPtr, std::nullptr_t>;
    std::variant<i64, std::pair<PE, PE>, std::nullptr_t> Raw;

  public:
    constexpr TableEntry(int V) : Raw(static_cast<i64>(V)) {}
    constexpr TableEntry(int Fst, FieldPtr Snd) : Raw(std::make_pair(static_cast<i32>(Fst), Snd)) {}
    constexpr TableEntry(std::nullptr_t, FieldPtr Snd) : Raw(std::make_pair(nullptr, Snd)) {}
    constexpr TableEntry(FieldPtr Fst, FieldPtr Snd) : Raw(std::make_pair(Fst, Snd)) {}
    constexpr TableEntry(std::nullptr_t) : Raw(nullptr) {}

    template <typename Record, typename Sz>
    constexpr void assertOrAssign(Record &R, const PE &P, Sz Bytes) const
    {
      if (std::holds_alternative<std::nullptr_t>(P))
        return;
      if (std::holds_alternative<FieldPtr>(P))
      {
        auto Field = std::get<FieldPtr>(P);
        R.*Field = Bytes;
        return;
      }
      util::assert_eq(std::get<i32>(P), Bytes);
    }

    template <typename Record>
    constexpr void assertOrAssign(Record &R, i64 DByte) const
    {
      if (std::holds_alternative<std::nullptr_t>(Raw))
        return;
      if (std::holds_alternative<std::pair<PE, PE>>(Raw))
      {
        auto &[RawFst, RawSnd] = std::get<std::pair<PE, PE>>(Raw);
        auto [BFst, BSnd] = util::le_pair(DByte);
        // little endian
        assertOrAssign(R, RawFst, BFst);
        assertOrAssign(R, RawSnd, BSnd);
        return;
      }
      util::assert_eq(std::get<i64>(Raw), util::le_extract(DByte));
    }
  };

  constexpr std::nullptr_t Skip = nullptr;

  constexpr TableEntry<i32 Page::*> PageTable[] = {
      0x7265, 0x4d61, 0x726b, 0x6162, 0x6c65, 0x202e, 0x6c69, 0x6e65, 0x7320, 0x6669, 0x6c65, 0x2c20, 0x7665, 0x7273, 0x696f, 0x6e3d, 0x3520, 0x2020, 0x2020, 0x2020, 0x2020, {0x20, &Page::NChildren}};

  constexpr TableEntry<i32 Layer::*> LayerTable[] = {0x0000, {0x00, &Layer::NChildren}};

  constexpr TableEntry<i32 Line::*> LineTable[] = {Skip, {Skip, &Line::BrushType}, {&Line::BrushColor, &Line::Padding}, {Skip, &Line::BrushSize}, Skip, Skip, Skip, Skip, Skip, Skip, Skip, {Skip, &Line::NChildren}};

  constexpr TableEntry<i32 Point::*> PointTable[] = {
      {&Point::X, &Point::Y}, {&Point::Speed, &Point::Direction}, {&Point::Width, &Point::Pressure}};
} // namespace fc::rM

namespace fc::rM::tablemap
{
  template <typename T>
  constexpr std::pair<TableEntry<i32 T::*> *, size_t> M;

  template <>
  constexpr inline auto M<Page> = std::make_pair(PageTable, std::extent_v<decltype(PageTable)>);
  template <>
  constexpr inline auto M<Layer> = std::make_pair(LayerTable, std::extent_v<decltype(LayerTable)>);
  template <>
  constexpr inline auto M<Line> = std::make_pair(LineTable, std::extent_v<decltype(LineTable)>);
  template <>
  constexpr inline auto M<Point> = std::make_pair(PointTable, std::extent_v<decltype(PointTable)>);
} // namespace fc::rM::tablemap
