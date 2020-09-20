#include <ostream>
#include <algorithm>

#include "Structures.hpp"

namespace fc::rM
{
  inline std::ostream &operator<<(std::ostream &OS, const Point &Obj)
  {
    OS << std::string(3, ' ') << "{" << reinterpret_cast<const float &>(Obj.X) << ", " << reinterpret_cast<const float &>(Obj.Y) << ", " << reinterpret_cast<const float &>(Obj.Speed) << ", " << reinterpret_cast<const float &>(Obj.Direction) << ", " << reinterpret_cast<const float &>(Obj.Width) << ", " << reinterpret_cast<const float &>(Obj.Pressure) << "}" << std::endl;
    std::for_each(Obj.Children.begin(), Obj.Children.end(), [&OS](auto Child) { OS << Child; });
    return OS;
  }
  inline std::ostream &operator<<(std::ostream &OS, const Line &Obj)
  {
    OS << std::string(2, ' ') << std::dec << +Obj.NChildren << " points:" << std::endl;
    std::for_each(Obj.Children.begin(), Obj.Children.end(), [&OS](auto Child) { OS << Child; });
    return OS;
  }
  inline std::ostream &operator<<(std::ostream &OS, const Layer &Obj)
  {
    OS << std::string(1, ' ') << std::dec << +Obj.NChildren << " lines:" << std::endl;
    std::for_each(Obj.Children.begin(), Obj.Children.end(), [&OS](auto Child) { OS << Child; });
    return OS;
  }
  inline std::ostream &operator<<(std::ostream &OS, const Page &Obj)
  {
    OS << std::dec << +Obj.NChildren << " layers:" << std::endl;
    std::for_each(Obj.Children.begin(), Obj.Children.end(), [&OS](auto Child) { OS << Child; });
    return OS;
  }
} // namespace fc::rM
