#include <iostream>
#include <fstream>
#include <cassert>

#include "SvgParse.hpp"

using namespace fc::svg;

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " <.svg file>" << std::endl;
    return EXIT_FAILURE;
  }
  std::ifstream Stream(argv[1], std::ios::binary);
  assert(Stream.is_open());

  auto image = parseFromFile(argv[1], "px", 96);
  std::cout << "Size: " << image->width << "x" << image->height << std::endl;
  for (Shape *shape = image->shapes; shape != nullptr; shape = shape->next)
  {
    for (Path *path = shape->paths; path != nullptr; path = path->next)
    {
      std::cout << "Path has " << path->npts << " points" << std::endl;
    }
    deleteImage(image);
  }
  return EXIT_SUCCESS;
}
