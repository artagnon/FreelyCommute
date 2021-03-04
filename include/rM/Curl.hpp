#include <cassert>
#include <curl/curl.h>

extern "C"
{
  CURL *curl_init()
  {
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *curl = curl_easy_init();
    assert(curl);
    return curl;
  }

  void curl_cleanup(CURL *curl)
  {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
  }

  int curl_post(CURL *curl, const char url[], const char postfields[])
  {
    /* First set the URL that is about to receive our POST. This URL can
      just as well be a https:// URL if that is what should receive the
      data. */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    /* Now specify the POST data */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields);

    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);

    /* Check for errors */
    if (res != CURLE_OK)
    {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
      return -1;
    }
    return 0;
  }
}
