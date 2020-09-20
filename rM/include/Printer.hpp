#include <ostream>
#include <algorithm>

#include "Structures.hpp"

namespace fc::rM
{
  inline std::ostream &operator<<(std::ostream &OS, const Point &Obj)
  {
    OS << "Point has " << std::dec << +Obj.NChildren << " children" << std::endl;
    std::for_each(Obj.Children.begin(), Obj.Children.end(), [&OS](auto Child) { OS << Child; });
    return OS;
  }
  inline std::ostream &operator<<(std::ostream &OS, const Line &Obj)
  {
    OS << "Line has " << std::dec << +Obj.NChildren << " children" << std::endl;
    std::for_each(Obj.Children.begin(), Obj.Children.end(), [&OS](auto Child) { OS << Child; });
    return OS;
  }
  inline std::ostream &operator<<(std::ostream &OS, const Layer &Obj)
  {
    OS << "Layer has " << std::dec << +Obj.NChildren << " children" << std::endl;
    std::for_each(Obj.Children.begin(), Obj.Children.end(), [&OS](auto Child) { OS << Child; });
    return OS;
  }
  inline std::ostream &operator<<(std::ostream &OS, const Page &Obj)
  {
    OS << "Page has " << std::dec << +Obj.NChildren << " children" << std::endl;
    std::for_each(Obj.Children.begin(), Obj.Children.end(), [&OS](auto Child) { OS << Child; });
    return OS;
  }
} // namespace fc::rM
