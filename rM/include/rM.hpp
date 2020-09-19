#pragma once

#include <array>
#include <utility>
#include <new>
#include <fstream>
#include <variant>

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

  enum class HdrE
  {
    NLayers,
    NLines,
    NPoints,
    BrushColor,
    BrushType,
    BrushBaseSize,
    PointX,
    PointY,
    PointSpeed,
    PointDirection,
    PointWidth,
    PointPressure
  };

  using PrimitiveEntry = std::variant<i32, HdrE>;
  using E2 = std::array<PrimitiveEntry, 2>;
  using TableEntry = std::variant<i64, E2, std::nullptr_t>;
  constexpr std::nullptr_t Ghost = nullptr;

  constexpr std::array<TableEntry, 16> SigTable = {
      0x3520, 0x2020, 0x2020, 0x2020, 0x2020, 0x2001, 0x0000, 0x0000, 0x3520, 0x2020, 0x2020, 0x2020, 0x2020, E2{0x20, HdrE::NLayers}, 0x0000, E2{0x00, HdrE::NLines}};

  constexpr std::array<TableEntry, 14> LineTable = {Ghost, E2{0x00, HdrE::BrushType}, Ghost, Ghost, Ghost, Ghost, Ghost, Ghost, Ghost, Ghost, Ghost, Ghost, Ghost, E2{0x00, HdrE::NPoints}};

  constexpr std::array<TableEntry, 3> PointTable = {
      E2{HdrE::PointX, HdrE::PointY}, E2{HdrE::PointSpeed, HdrE::PointDirection}, E2{HdrE::PointWidth, HdrE::PointPressure}};

  using bytestream = std::basic_ifstream<i32>;

  class ParseData
  {
    unsigned int NLayers;
    unsigned int NLines;
  };

  class Parser
  {
    ParseData Data;

  public:
    Parser(bytestream &Stream);
    operator std::string();
  };
} // namespace fc::rM
