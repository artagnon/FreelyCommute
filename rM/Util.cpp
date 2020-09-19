#include <string>
#include <iostream>

#include "Util.hpp"

namespace fc::rM::util
{
  void assert(bool Condition, std::string Msg)
  {
    if (!Condition)
    {
      std::cout << Msg << std::endl;
      std::abort();
    }
  }
} // namespace fc::rM::util
