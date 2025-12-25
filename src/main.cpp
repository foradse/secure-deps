#include "allowlist.h"
#include "cmake_fetchcontent.h"
#include "makefile_parser.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace {

bool ReadTextFile(std::string path, std::string* out, std::string* error) {
  std::ifstream file(path, std::ios::in | std::ios::binary);
  if (!file) {
    if (error) {
      *error = "Failed to open file: " + path;
    }
    return false;
  }
  std::ostringstream buffer;
  buffer << file.rdbuf();
  *out = buffer.str();
  return true;
}

bool WriteTextFile(std::string path, std::string content, std::string* error) {
  std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
  if (!file) {
    if (error) {
      *error = "Failed to write file: " + path;
    }
    return false;
  }
  file << content;
  return true;
}

std::vector<std::string> UniquePreserveOrder(std::vector<std::string> values) {
  std::unordered_set<std::string> seen;
  std::vector<std::string> unique;
  for (auto& value : values) {
    if (seen.insert(value).second) {
      unique.push_back(value);
    }
  }
  return unique;
}

std::string JsonEscape(std::string value) {
  std::string out;
  out.reserve(value.size());
  for (unsigned char ch : value) {
    switch (ch) {
      case '\"':
        out += "\\\"";
        break;
      case '\\':
        out += "\\\\";
        break;
      case '\b':
        out += "\\b";
        break;
      case '\f':
        out += "\\f";
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        if (ch < 0x20) {
          char hex[] = "0123456789abcdef";
          out += "\\u00";
          out.push_back(hex[(ch >> 4) & 0x0f]);
          out.push_back(hex[ch & 0x0f]);
        } else {
          out.push_back(static_cast<char>(ch));
        }
        break;
    }
  }
  return out;
}

std::string JsonArray(std::vector<std::string> values) {
  std::ostringstream out;
  out << "[";
  for (size_t i = 0; i < values.size(); ++i) {
    if (i > 0) {
      out << ",";
    }
    out << "\"" << JsonEscape(values[i]) << "\"";
  }
  out << "]";
  return out.str();
}

void PrintUsage() {
  std::cout << "Usage:\n"
            << "  secure_deps makefile-check --makefile <path> [--allowlist <path>] [--json]\n"
            << "  secure_deps cmake-rewrite --input <path> --allowlist <path> [--output <path>] [--json]\n";
}

int RunMakefileCheck(int argc, char** argv) {
  std::string makefile_path = "Makefile";
  std::string allowlist_path;
  bool json_output = false;

  for (int i = 2; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--makefile" && i + 1 < argc) {
      makefile_path = argv[++i];
    } else if (arg == "--allowlist" && i + 1 < argc) {
      allowlist_path = argv[++i];
    } else if (arg == "--json") {
      json_output = true;
    } else if (arg == "--help") {
      PrintUsage();
      return 0;
    } else {
      std::cerr << "Unknown argument: " << arg << "\n";
      PrintUsage();
      return 2;
    }
  }

  std::string makefile_content;
  std::string error;
  if (!ReadTextFile(makefile_path, &makefile_content, &error)) {
    std::cerr << error << "\n";
    return 2;
  }

  std::vector<std::string> libraries = UniquePreserveOrder(ExtractLibrariesFromMakefile(makefile_content));

  std::vector<std::string> disallowed;
  if (!allowlist_path.empty()) {
    std::string allowlist_content;
    if (!ReadTextFile(allowlist_path, &allowlist_content, &error)) {
      std::cerr << error << "\n";
      return 2;
    }
    auto allowlist = ParseAllowlist(allowlist_content);
    for (auto& lib : libraries) {
      if (allowlist.find(lib) == allowlist.end()) {
        disallowed.push_back(lib);
      }
    }
  }

  if (json_output) {
    std::cout << "{"
              << "\"libraries\":" << JsonArray(libraries) << ","
              << "\"disallowed\":" << JsonArray(disallowed) << ","
              << "\"status\":\"" << (disallowed.empty() ? "ok" : "fail") << "\""
              << "}\n";
  } else {
    std::cout << "Libraries found (" << libraries.size() << "):\n";
    for (auto& lib : libraries) {
      std::cout << "  " << lib << "\n";
    }
    if (!allowlist_path.empty()) {
      if (!disallowed.empty()) {
        std::cout << "Disallowed libraries (" << disallowed.size() << "):\n";
        for (auto& lib : disallowed) {
          std::cout << "  " << lib << "\n";
        }
      } else {
        std::cout << "All libraries are allowed.\n";
      }
    }
  }

  return disallowed.empty() ? 0 : 1;
}

int RunCmakeRewrite(int argc, char** argv) {
  std::string input_path = "CMakeLists.txt";
  std::string output_path;
  std::string allowlist_path;
  bool json_output = false;

  for (int i = 2; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--input" && i + 1 < argc) {
      input_path = argv[++i];
    } else if (arg == "--output" && i + 1 < argc) {
      output_path = argv[++i];
    } else if (arg == "--allowlist" && i + 1 < argc) {
      allowlist_path = argv[++i];
    } else if (arg == "--json") {
      json_output = true;
    } else if (arg == "--help") {
      PrintUsage();
      return 0;
    } else {
      std::cerr << "Unknown argument: " << arg << "\n";
      PrintUsage();
      return 2;
    }
  }

  if (allowlist_path.empty()) {
    std::cerr << "--allowlist is required for cmake-rewrite.\n";
    return 2;
  }

  std::string cmake_content;
  std::string error;
  if (!ReadTextFile(input_path, &cmake_content, &error)) {
    std::cerr << error << "\n";
    return 2;
  }

  std::string allowlist_content;
  if (!ReadTextFile(allowlist_path, &allowlist_content, &error)) {
    std::cerr << error << "\n";
    return 2;
  }
  auto repo_map = ParseRepoMap(allowlist_content);
  RepoReplaceResult result = ReplaceFetchContentRepos(cmake_content, repo_map);

  if (!result.missing.empty()) {
    if (json_output) {
      std::cout << "{"
                << "\"replaced\":" << JsonArray(result.replaced) << ","
                << "\"missing\":" << JsonArray(result.missing) << ","
                << "\"status\":\"fail\""
                << "}\n";
    } else {
      std::cerr << "Repositories not in allowlist (" << result.missing.size() << "):\n";
      for (auto& repo : result.missing) {
        std::cerr << "  " << repo << "\n";
      }
    }
    return 1;
  }

  if (output_path.empty()) {
    output_path = input_path;
  }
  if (!WriteTextFile(output_path, result.output, &error)) {
    std::cerr << error << "\n";
    return 2;
  }

  if (json_output) {
    std::cout << "{"
              << "\"replaced\":" << JsonArray(result.replaced) << ","
              << "\"missing\":[],"
              << "\"status\":\"ok\""
              << "}\n";
  } else {
    if (!result.replaced.empty()) {
      std::cout << "Replaced repositories (" << result.replaced.size() << "):\n";
      for (auto& entry : result.replaced) {
        std::cout << "  " << entry << "\n";
      }
    } else {
      std::cout << "No repositories replaced.\n";
    }
  }
  return 0;
}

}  // анонимное пространство имен

int main(int argc, char** argv) {
  if (argc < 2) {
    PrintUsage();
    return 2;
  }

  std::string command = argv[1];
  if (command == "makefile-check") {
    return RunMakefileCheck(argc, argv);
  }
  if (command == "cmake-rewrite") {
    return RunCmakeRewrite(argc, argv);
  }

  PrintUsage();
  return 2;
}
