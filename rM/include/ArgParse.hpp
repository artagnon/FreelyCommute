#include <algorithm>
#include <vector>
#include <string>

class Parser
{
  std::string Argv0;
  std::vector<std::string> Args;

  Parser(int argc, char *argv[])
  {
    Argv0 = argv[0];
    for (unsigned i = 1; i < argc; i++)
    {
      Args.emplace_back(argv[i]);
    }
  }
};
