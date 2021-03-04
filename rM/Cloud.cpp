#include "rM/Cloud.hpp"

using namespace fc::rM;

int main()
{
  Curl C;
  StrMap PostArgs = {{"username", "artagnon"}, {"password", "foo"}};
  C.post("https://my.remarkable.com/", PostArgs);
  return EXIT_SUCCESS;
}
