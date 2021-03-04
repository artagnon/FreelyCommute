#pragma once

#include <string>
#include <vector>
#include <map>
#include <numeric>

#include "Curl.hpp"

namespace fc::rM
{
  using StrMap = std::map<std::string, std::string>;
  class Curl
  {
    CURL *curl;

  public:
    Curl() : curl(curl_init()) {}
    ~Curl() { curl_cleanup(curl); }

    int post(std::string Url, StrMap M)
    {
      const std::string JPost = std::accumulate(M.begin(), M.end(), std::string{},
                                                [](const std::string &s, const std::pair<const std::string, std::string> &p) {
                                                  return s + (s.empty() ? std::string() : "&") + p.first;
                                                });
      return curl_post(curl, Url.c_str(), JPost.c_str());
    }
  };
}
