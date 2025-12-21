#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct RepoReplaceResult {
  std::string output;
  std::vector<std::string> replaced;
  std::vector<std::string> missing;
};

RepoReplaceResult ReplaceFetchContentRepos(
    const std::string& content,
    const std::unordered_map<std::string, std::string>& allowed_map);
