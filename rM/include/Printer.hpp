#include <ostream>

#include "Structures.hpp"

#define check_err(condition) condition ? void(0) : SpecialType &ErrorStreamer

namespace fc::rM
{
  template <typename T>
  inline void printChildren(std::ostream &OS, const T &Obj)
  {
    std::for_each(Obj.Children.begin(), Obj.Children.end(), [&OS](auto Child) { OS << Child; });
  }
  inline std::ostream &operator<<(std::ostream &OS, const Point &Obj)
  {
    OS << std::string(3, ' ') << "{" << Obj.X << ", " << Obj.Y << ", " << Obj.Speed << ", " << Obj.Direction << ", " << Obj.Width << ", " << Obj.Pressure << "}" << std::endl;
    return OS;
  }
  inline std::ostream &operator<<(std::ostream &OS, const Line &Obj)
  {
    OS << std::string(2, ' ') << std::dec << Obj.NChildren << " points:" << std::endl;
    printChildren(OS, Obj);
    return OS;
  }
  inline std::ostream &operator<<(std::ostream &OS, const Layer &Obj)
  {
    OS << std::string(1, ' ') << std::dec << Obj.NChildren << " lines:" << std::endl;
    printChildren(OS, Obj);
    return OS;
  }
  inline std::ostream &operator<<(std::ostream &OS, const Page &Obj)
  {
    OS << std::dec << +Obj.NChildren << " layers:" << std::endl;
    printChildren(OS, Obj);
    return OS;
  }
  class ErrorStreamer
  {
  };

  inline std::ostream &operator<<(std::ostream &OS, const ErrorStreamer &Obj)
  {
  }
} // namespace fc::rM
