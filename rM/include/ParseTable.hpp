#pragma once

#include <utility>
#include <new>
#include <fstream>
#include <variant>
#include <cassert>

#include "Utility.hpp"
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
    using PE = std::variant<i32, FieldPtr>;
    std::variant<i64, std::pair<PE, PE>> Raw;

  public:
    constexpr TableEntry(int V) : Raw(static_cast<i64>(V)) {}
    constexpr TableEntry(int Fst, FieldPtr Snd) : Raw(std::make_pair(static_cast<i32>(Fst), Snd)) {}
    constexpr TableEntry(FieldPtr Fst, FieldPtr Snd) : Raw(std::make_pair(Fst, Snd)) {}
    constexpr TableEntry(std::nullptr_t) {}

    template <typename Record, typename Sz>
    void assertOrAssign(Record &R, const PE &P, Sz Bytes) const
    {
      if (std::holds_alternative<FieldPtr>(P))
      {
        auto Field = std::get<FieldPtr>(P);
        R.*Field = Bytes;
        return;
      }
      assert(std::get<i32>(P) == Bytes);
    }

    template <typename Record>
    void assertOrAssign(Record &R, i64 DByte) const
    {
      if (std::holds_alternative<std::pair<PE, PE>>(Raw))
      {
        auto &[RawFst, RawSnd] = std::get<std::pair<PE, PE>>(Raw);
        assertOrAssign(R, RawFst, DByte % 0x100);
        assertOrAssign(R, RawSnd, DByte / 0x100);
        return;
      }
      assert(std::get<i64>(Raw) == DByte);
    }
  };

  constexpr std::nullptr_t Ghost = nullptr;

  constexpr TableEntry<i32 Page::*> PageTable[15] = {
      0x3520, 0x2020, 0x2020, 0x2020, 0x2020, 0x2001, 0x0000, 0x0000, 0x3520, 0x2020, 0x2020, 0x2020, 0x2020, {0x20, &Page::NChildren}, 0x0000};

  constexpr TableEntry<i32 Layer::*> LayerTable[1] = {{0x00, &Layer::NChildren}};

  constexpr TableEntry<i32 Line::*> LineTable[14] = {Ghost, {0x00, &Line::BrushType}, Ghost, Ghost, Ghost, Ghost, Ghost, Ghost, Ghost, Ghost, Ghost, Ghost, Ghost, {0x00, &Line::NChildren}};

  constexpr TableEntry<i32 Point::*> PointTable[3] = {
      {&Point::X, &Point::Y}, {&Point::Speed, &Point::Direction}, {&Point::Width, &Point::Pressure}};
} // namespace fc::rM

namespace fc::rM::tablemap
{
  template <typename T>
  std::pair<TableEntry<i32 T::*> *, size_t> M;
}