#include "allowlist.h"

#include <cctype>
#include <sstream>

namespace {

std::string Trim(std::string value) {
  size_t start = 0;
  while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
    ++start;
  }
  size_t end = value.size();
  while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
    --end;
  }
  return value.substr(start, end - start);
}

std::string StripQuotes(std::string value) {
  if (value.size() >= 2) {
    char first = value.front();
    char last = value.back();
    if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
      return value.substr(1, value.size() - 2);
    }
  }
  return value;
}

std::string StripInlineComment(std::string line) {
  bool in_single = false;
  bool in_double = false;
  for (size_t i = 0; i < line.size(); ++i) {
    char ch = line[i];
    if (ch == '\'' && !in_double) {
      in_single = !in_single;
    } else if (ch == '"' && !in_single) {
      in_double = !in_double;
    } else if (ch == '#' && !in_single && !in_double) {
      return line.substr(0, i);
    }
  }
  return line;
}

std::string NormalizeLibName(std::string name) {
  std::string normalized = name;
  if (normalized.rfind("-l", 0) == 0) {
    normalized = normalized.substr(2);
  }
  if (!normalized.empty() && normalized.front() == ':') {
    normalized.erase(0, 1);
  }
  return normalized;
}

}

std::unordered_set<std::string> ParseAllowlist(std::string content) {
  std::unordered_set<std::string> allowlist;
  std::istringstream stream(content);
  std::string line;
  while (std::getline(stream, line)) {
    line = StripInlineComment(line);
    line = Trim(line);
    if (line.empty()) {
      continue;
    }
    allowlist.insert(NormalizeLibName(StripQuotes(line)));
  }
  return allowlist;
}

std::unordered_map<std::string, std::string> ParseRepoMap(std::string content) {
  std::unordered_map<std::string, std::string> repo_map;
  std::istringstream stream(content);
  std::string line;
  while (std::getline(stream, line)) {
    line = StripInlineComment(line);
    line = Trim(line);
    if (line.empty()) {
      continue;
    }
    size_t arrow = line.find("=>");
    if (arrow == std::string::npos) {
      continue;
    }
    std::string left = Trim(line.substr(0, arrow));
    std::string right = Trim(line.substr(arrow + 2));
    left = StripQuotes(left);
    right = StripQuotes(right);
    if (!left.empty() && !right.empty()) {
      repo_map[left] = right;
    }
  }
  return repo_map;
}
