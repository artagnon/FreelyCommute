#include <ostream>

#include "Util.hpp"
#include "Structures.hpp"

namespace fc::rM
{
  inline std::ostream &operator<<(std::ostream &OS, const Point &Obj)
  {
    OS << "Point has " << +Obj.NChildren << " children" << std::endl;
    util::for_each(Obj.Children, [&OS](auto Child) { OS << Child; });
    return OS;
  }
  inline std::ostream &operator<<(std::ostream &OS, const Line &Obj)
  {
    OS << "Line has " << +Obj.NChildren << " children" << std::endl;
    util::for_each(Obj.Children, [&OS](auto Child) { OS << Child; });
    return OS;
  }
  inline std::ostream &operator<<(std::ostream &OS, const Layer &Obj)
  {
    OS << "Layer has " << +Obj.NChildren << " children" << std::endl;
    util::for_each(Obj.Children, [&OS](auto Child) { OS << Child; });
    return OS;
  }
  inline std::ostream &operator<<(std::ostream &OS, const Page &Obj)
  {
    OS << "Page has " << +Obj.NChildren << " children" << std::endl;
    util::for_each(Obj.Children, [&OS](auto Child) { OS << Child; });
    return OS;
  }
} // namespace fc::rM
