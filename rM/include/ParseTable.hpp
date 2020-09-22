#pragma once

#include <new>
#include <fstream>

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
    using iFieldPtr = i32 Record::*;
    using fFieldPtr = f32 Record::*;
    using Variant = std::variant<iFieldPtr, fFieldPtr>;

    Variant Ptr;

  public:
    constexpr TableEntry(iFieldPtr V) : Ptr(V) {}
    constexpr TableEntry(fFieldPtr V) : Ptr(V) {}
    constexpr void assign(Record &R, i32 Bytes) const
    {
      std::visit([&R, &Bytes](auto Arg) {
        if constexpr (util::decay_equiv_v<decltype(Arg), iFieldPtr>)
          R.*(Arg) = util::to_le(reinterpret_cast<char *>(&Bytes));
        else if constexpr (util::decay_equiv_v<decltype(Arg), fFieldPtr>)
          R.*(Arg) = reinterpret_cast<f32 &>(Bytes);
      },
                 Ptr);
    }
  };

  constexpr TableEntry<Page> PageTable[] = {&Page::NChildren};

  constexpr TableEntry<Layer> LayerTable[] = {&Layer::NChildren};

  constexpr TableEntry<Line> LineTable[] = {&Line::BrushType, &Line::BrushColor,
                                            &Line::Padding, &Line::Unknown, &Line::BrushSize, &Line::NChildren};

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
