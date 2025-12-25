#include "makefile_parser.h"

#include <regex>

namespace {

std::string StripMakefileComments(std::string content) {
  std::string out;
  out.reserve(content.size());
  bool in_comment = false;
  for (char ch : content) {
    if (in_comment) {
      if (ch == '\n') {
        in_comment = false;
        out.push_back(ch);
      } else {
        out.push_back(' ');
      }
      continue;
    }
    if (ch == '#') {
      in_comment = true;
      out.push_back(' ');
      continue;
    }
    out.push_back(ch);
  }
  return out;
}

std::string NormalizeLibraryToken(std::string token) {
  if (!token.empty() && token.front() == ':') {
    return token.substr(1);
  }
  return token;
}

}  // анонимное пространство имен

std::vector<std::string> ExtractLibrariesFromMakefile(std::string content) {
  std::string sanitized = StripMakefileComments(content);
  std::regex pattern(R"(-l(:?[A-Za-z0-9_./+\-]+))");
  std::vector<std::string> libraries;
  for (std::sregex_iterator it(sanitized.begin(), sanitized.end(), pattern);
       it != std::sregex_iterator();
       ++it) {
    std::string value = (*it)[1].str();
    libraries.push_back(NormalizeLibraryToken(value));
  }
  return libraries;
}
