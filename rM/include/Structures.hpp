#pragma once

#include <deque>
#include <ostream>

namespace fc::rM
{
  using i32 = uint32_t;
  using f32 = uint32_t;

  template <typename T>
  struct AttachChildren
  {
    i32 NChildren = 0;
    std::deque<T> Children;
  };

  struct Point : public AttachChildren<std::nullptr_t>
  {
    f32 X, Y, Speed, Direction, Width, Pressure;
  };

  struct Line : public AttachChildren<Point>
  {
    i32 BrushType, BrushColor, Padding, Unknown;
    f32 BrushSize;
  };

  struct Layer : public AttachChildren<Line>
  {
  };
  struct Page : public AttachChildren<Layer>
  {
  };
} // namespace fc::rM
