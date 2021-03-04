#pragma once

#include <cctype>

namespace fc::svg
{
#define _XML_TAG 1
#define _XML_CONTENT 2
#define _XML_MAX_ATTRIBS 256

  static void parseContent(char *s,
                           void (*contentCb)(void *ud, const char *s),
                           void *ud)
  {
    // Trim start white spaces
    while (*s && std::isspace(*s))
      s++;
    if (!*s)
      return;

    if (contentCb)
      (*contentCb)(ud, s);
  }

  static void parseElement(char *s,
                           void (*startelCb)(void *ud, const char *el, const char **attr),
                           void (*endelCb)(void *ud, const char *el),
                           void *ud)
  {
    const char *attr[_XML_MAX_ATTRIBS];
    int nattr = 0;
    char *name;
    int start = 0;
    int end = 0;
    char quote;

    // Skip white space after the '<'
    while (*s && std::isspace(*s))
      s++;

    // Check if the tag is end tag
    if (*s == '/')
    {
      s++;
      end = 1;
    }
    else
    {
      start = 1;
    }

    // Skip comments, data and preprocessor stuff.
    if (!*s || *s == '?' || *s == '!')
      return;

    // Get tag name
    name = s;
    while (*s && !std::isspace(*s))
      s++;
    if (*s)
    {
      *s++ = '\0';
    }

    // Get attribs
    while (!end && *s && nattr < _XML_MAX_ATTRIBS - 3)
    {
      char *name = nullptr;
      char *value = nullptr;

      // Skip white space before the attrib name
      while (*s && std::isspace(*s))
        s++;
      if (!*s)
        break;
      if (*s == '/')
      {
        end = 1;
        break;
      }
      name = s;
      // Find end of the attrib name.
      while (*s && !std::isspace(*s) && *s != '=')
        s++;
      if (*s)
      {
        *s++ = '\0';
      }
      // Skip until the beginning of the value.
      while (*s && *s != '\"' && *s != '\'')
        s++;
      if (!*s)
        break;
      quote = *s;
      s++;
      // Store value and find the end of it.
      value = s;
      while (*s && *s != quote)
        s++;
      if (*s)
      {
        *s++ = '\0';
      }

      // Store only well formed attributes
      if (name && value)
      {
        attr[nattr++] = name;
        attr[nattr++] = value;
      }
    }

    // List terminator
    attr[nattr++] = 0;
    attr[nattr++] = 0;

    // Call callbacks.
    if (start && startelCb)
      (*startelCb)(ud, name, attr);
    if (end && endelCb)
      (*endelCb)(ud, name);
  }

  int parseXML(char *input,
               void (*startelCb)(void *ud, const char *el, const char **attr),
               void (*endelCb)(void *ud, const char *el),
               void (*contentCb)(void *ud, const char *s),
               void *ud)
  {
    char *s = input;
    char *mark = s;
    int state = _XML_CONTENT;
    while (*s)
    {
      if (*s == '<' && state == _XML_CONTENT)
      {
        // Start of a tag
        *s++ = '\0';
        parseContent(mark, contentCb, ud);
        mark = s;
        state = _XML_TAG;
      }
      else if (*s == '>' && state == _XML_TAG)
      {
        // Start of a content or new tag.
        *s++ = '\0';
        parseElement(mark, startelCb, endelCb, ud);
        mark = s;
        state = _XML_CONTENT;
      }
      else
      {
        s++;
      }
    }

    return 1;
  }
}
