#pragma once

namespace fc::rM
{
  using i32 = uint32_t; // We do this to aid reading the binary stream
  using f32 = uint32_t; // Anyway, we're parsing one byte that we expect to reinterpret_cast appropriately later

  // It's important that we write out these structures in full, because we need padding/alignment data
  // The .rM is a dump of padded structures, to aid quick serialization/deserialization
  // Do not try to replace C-style arrays with any fancy data structures: the padding data will get messed up
  struct Point
  {
    f32 X, Y, Speed, Direction, Width, Pressure;
  };

  struct Line
  {
    i32 BrushType;
    i32 BrushColor;
    i32 Padding;
    f32 Unknown;
    f32 BrushSize;
    i32 NChildren;
    Point Children[];
  };

  struct Layer
  {
    i32 NChildren;
    Line Children[];
  };

  struct Page
  {
    i32 NChildren;
    Layer Children[];
  };
} // namespace fc::rM
