#pragma once

#include <vector>

namespace fc::rM
{
  using i32 = uint8_t;
  using i64 = uint16_t;

  template <typename T>
  struct AttachChildren
  {
    i32 NChildren;
    std::vector<T> Children;
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
} // namespace fc::rM