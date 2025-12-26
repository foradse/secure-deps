#include "cmake_fetchcontent.h"

#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace {

enum class TokenType {
  kWord,
  kString,
  kPunct,
};

struct Token {
  TokenType type;
  std::string text;
  size_t start = 0;
  size_t end = 0;
  char quote = 0;
};

std::string ToLower(std::string value) {
  std::string out = value;
  std::transform(out.begin(), out.end(), out.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return out;
}

std::vector<Token> TokenizeCMake(std::string content) {
  std::vector<Token> tokens;
  size_t i = 0;
  while (i < content.size()) {
    char ch = content[i];
    if (std::isspace(static_cast<unsigned char>(ch))) {
      ++i;
      continue;
    }
    if (ch == '#') {
      while (i < content.size() && content[i] != '\n') {
        ++i;
      }
      continue;
    }
    if (ch == '\'' || ch == '"') {
      char quote = ch;
      size_t start = i;
      ++i;
      while (i < content.size()) {
        char current = content[i];
        if (current == '\\' && i + 1 < content.size()) {
          i += 2;
          continue;
        }
        if (current == quote) {
          ++i;
          break;
        }
        ++i;
      }
      size_t end = i;
      std::string inner;
      if (end > start + 1) {
        inner = content.substr(start + 1, end - start - 2);
      }
      tokens.push_back({TokenType::kString, inner, start, end, quote});
      continue;
    }
    if (ch == '(' || ch == ')') {
      size_t start = i;
      ++i;
      tokens.push_back({TokenType::kPunct, std::string(1, ch), start, i, 0});
      continue;
    }
    size_t start = i;
    while (i < content.size()) {
      char current = content[i];
      if (std::isspace(static_cast<unsigned char>(current)) || current == '(' || current == ')' || current == '#') {
        break;
      }
      ++i;
    }
    if (i > start) {
      tokens.push_back({TokenType::kWord, content.substr(start, i - start), start, i, 0});
    }
  }
  return tokens;
}

size_t FindMatchingParen(std::vector<Token> tokens, size_t open_index) {
  if (open_index >= tokens.size()) {
    return std::string::npos;
  }
  if (tokens[open_index].type != TokenType::kPunct || tokens[open_index].text != "(") {
    return std::string::npos;
  }
  int depth = 0;
  for (size_t i = open_index; i < tokens.size(); ++i) {
    if (tokens[i].type != TokenType::kPunct) {
      continue;
    }
    if (tokens[i].text == "(") {
      ++depth;
    } else if (tokens[i].text == ")") {
      --depth;
      if (depth == 0) {
        return i;
      }
    }
  }
  return std::string::npos;
}

void AddUnique(std::vector<std::string>* out, std::unordered_set<std::string>* seen,
               std::string value) {
  if (seen->insert(value).second) {
    out->push_back(value);
  }
}

}

RepoReplaceResult ReplaceFetchContentRepos(
    std::string content,
    std::unordered_map<std::string, std::string> allowed_map) {
  std::vector<Token> tokens = TokenizeCMake(content);
  struct Replacement {
    size_t start;
    size_t end;
    std::string new_text;
    std::string original;
    std::string replaced;
  };
  std::vector<Replacement> replacements;
  std::vector<std::string> replaced;
  std::vector<std::string> missing;
  std::unordered_set<std::string> replaced_seen;
  std::unordered_set<std::string> missing_seen;

  for (size_t i = 0; i < tokens.size(); ++i) {
    if (tokens[i].type != TokenType::kWord) {
      continue;
    }
    if (ToLower(tokens[i].text) != "fetchcontent_declare") {
      continue;
    }
    if (i + 1 >= tokens.size() || tokens[i + 1].type != TokenType::kPunct || tokens[i + 1].text != "(") {
      continue;
    }
    size_t call_end = FindMatchingParen(tokens, i + 1);
    if (call_end == std::string::npos) {
      continue;
    }
    for (size_t j = i + 1; j < call_end; ++j) {
      if (tokens[j].type != TokenType::kWord) {
        continue;
      }
      if (ToLower(tokens[j].text) != "git_repository") {
        continue;
      }
      size_t value_index = j + 1;
      while (value_index < call_end &&
             tokens[value_index].type == TokenType::kPunct &&
             tokens[value_index].text == "(") {
        ++value_index;
      }
      if (value_index >= call_end) {
        continue;
      }
      if (tokens[value_index].type != TokenType::kWord &&
          tokens[value_index].type != TokenType::kString) {
        continue;
      }
      Token value_token = tokens[value_index];
      std::string original = value_token.text;
      auto allowed_it = allowed_map.find(original);
      if (allowed_it == allowed_map.end()) {
        AddUnique(&missing, &missing_seen, original);
        continue;
      }
      std::string replacement = allowed_it->second;
      std::string new_text = replacement;
      if (value_token.type == TokenType::kString) {
        new_text = std::string(1, value_token.quote) + replacement + std::string(1, value_token.quote);
      }
      replacements.push_back({value_token.start, value_token.end, new_text, original, replacement});
      AddUnique(&replaced, &replaced_seen, original + " => " + replacement);
    }
  }

  std::string output = content;
  for (auto it = replacements.rbegin(); it != replacements.rend(); ++it) {
    output.replace(it->start, it->end - it->start, it->new_text);
  }

  return RepoReplaceResult{output, replaced, missing};
}
