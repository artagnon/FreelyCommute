#pragma once

#include <deque>

namespace fc::rM
{
  using i32 = uint32_t;
  using f32 = float;

  template <typename T>
  struct AttachChildren : public std::deque<T>
  {
    i32 NChildren = 0;
  };

  struct Point
  {
    f32 X, Y, Speed, Direction, Width, Pressure;
  };

  enum BrushType
  {
    BallPoint = 15,
    Marker = 16,
    Fineliner = 17,
    SharpPencil = 13,
    TiltPencil = 14,
    Brush = 12,
    Highlighter = 18,
    Eraser = 6,
    EraseArea = 8,
  };

  struct Line : public AttachChildren<Point>
  {
    i32 BrushType, BrushColor, Padding;
    f32 Unknown, BrushSize;
  };

  struct Layer : public AttachChildren<Line>
  {
  };
  struct Page : public AttachChildren<Layer>
  {
  };
}
