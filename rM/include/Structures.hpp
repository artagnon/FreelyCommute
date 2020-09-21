#pragma once

#include <vector>
#include <ostream>

namespace fc::rM
{
  using i32 = uint8_t;
  using i64 = uint16_t;

  template <typename T>
  struct AttachChildren
  {
    i32 NChildren = 0;
    std::vector<T> Children;
  };

  struct Point : public AttachChildren<std::nullptr_t>
  {
    i32 X, Y, Speed, Direction, Width, Pressure;
  };

  struct Line : public AttachChildren<Point>
  {
    i32 BrushType, BrushColor, Padding, BrushSize;
  };

  struct Layer : public AttachChildren<Line>
  {
  };
  struct Page : public AttachChildren<Layer>
  {
  };
} // namespace fc::rM
