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
  template <typename Record>
  class TableEntry
  {
    using FieldPtr = i32 Record::*;
    using Skips = std::pair<std::nullptr_t, size_t>;
    std::variant<i32, FieldPtr, Skips> Variant;

  public:
    constexpr TableEntry(int V) : Variant(static_cast<i32>(V)) {}
    constexpr TableEntry(std::pair<std::nullptr_t, std::size_t> V) : Variant(V) {}
    constexpr TableEntry(FieldPtr V) : Variant(V) {}

    constexpr size_t toSkip() const
    {
      return std::holds_alternative<Skips>(Variant) ? std::get<size_t>(std::get<Skips>(Variant)) : 0;
    }
    constexpr void assign(Record &R, i32 Byte) const
    {
      auto Visitor = [&R, &Byte](auto &Arg) {
        if constexpr (util::decay_equiv_v<decltype(Arg), FieldPtr>)
          R.*(Arg) = Byte;
        else if constexpr (util::decay_equiv_v<decltype(Arg), i32>)
          util::assert_eq(Arg, Byte);
      };
      std::visit(Visitor, Variant);
    }
  };

  template <size_t N>
  constexpr std::pair<std::nullptr_t, size_t> Skip = std::make_pair(nullptr, N);

  constexpr TableEntry<Page> PageTable[] = {
      0x72, 0x65, 0x4d, 0x61, 0x72, 0x6b, 0x61, 0x62, 0x6c, 0x65, 0x20, 0x2e, 0x6c, 0x69, 0x6e, 0x65, 0x73, 0x20, 0x66, 0x69, 0x6c, 0x65, 0x2c, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x3d, 0x35, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
      &Page::NChildren, 0x00, 0x00, 0x00};

  constexpr TableEntry<Layer> LayerTable[] = {&Layer::NChildren};

  constexpr TableEntry<Line> LineTable[] = {Skip<3>, &Line::BrushType, &Line::BrushColor,
                                            &Line::Padding, Skip<1>, &Line::BrushSize, Skip<15>, &Line::NChildren};

  constexpr TableEntry<Point> PointTable[] = {
      &Point::X, &Point::Y, &Point::Speed, &Point::Direction, &Point::Width, &Point::Pressure};
} // namespace fc::rM

namespace fc::rM::tablemap
{
  template <typename T>
  constexpr std::pair<TableEntry<i32 T::*> *, size_t> M;

  template <>
  constexpr inline auto &M<Page> = PageTable;
  template <>
  constexpr inline auto &M<Layer> = LayerTable;
  template <>
  constexpr inline auto &M<Line> = LineTable;
  template <>
  constexpr inline auto &M<Point> = PointTable;
} // namespace fc::rM::tablemap
