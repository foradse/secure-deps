#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>

std::unordered_set<std::string> ParseAllowlist(std::string content);
std::unordered_map<std::string, std::string> ParseRepoMap(std::string content);
