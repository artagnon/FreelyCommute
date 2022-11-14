#pragma once

#include "Util.hpp"
#include "Structures.hpp"

namespace fc::rM
{
  template <typename Record>
  class TableEntry
  {
    using iFieldPtr = i32 Record::*;
    using fFieldPtr = f32 Record::*;
    using Variant = std::variant<iFieldPtr, fFieldPtr>;
    const Variant Ptr;

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
