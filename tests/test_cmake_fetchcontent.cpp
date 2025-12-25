#include "cmake_fetchcontent.h"

#include <gtest/gtest.h>

#include <unordered_map>

TEST(CmakeFetchContent, ReplacesQuotedRepository) {
  std::string content =
      "FetchContent_Declare(\n"
      "  mylib\n"
      "  GIT_REPOSITORY \"https://github.com/orig/repo.git\"\n"
      ")\n";
  std::unordered_map<std::string, std::string> map = {
      {"https://github.com/orig/repo.git", "https://mirror.local/repo.git"},
  };

  RepoReplaceResult result = ReplaceFetchContentRepos(content, map);

  EXPECT_NE(result.output.find("https://mirror.local/repo.git"), std::string::npos);
  ASSERT_EQ(result.replaced.size(), 1u);
  EXPECT_EQ(result.replaced[0], "https://github.com/orig/repo.git => https://mirror.local/repo.git");
  EXPECT_TRUE(result.missing.empty());
}

TEST(CmakeFetchContent, ReplacesUnquotedRepository) {
  std::string content =
      "FetchContent_Declare(lib GIT_REPOSITORY https://github.com/orig/repo.git)\n";
  std::unordered_map<std::string, std::string> map = {
      {"https://github.com/orig/repo.git", "https://mirror.local/repo.git"},
  };

  RepoReplaceResult result = ReplaceFetchContentRepos(content, map);

  EXPECT_NE(result.output.find("https://mirror.local/repo.git"), std::string::npos);
  EXPECT_TRUE(result.missing.empty());
}

TEST(CmakeFetchContent, ReportsMissingRepository) {
  std::string content =
      "FetchContent_Declare(lib GIT_REPOSITORY \"https://example.com/missing.git\")\n";
  std::unordered_map<std::string, std::string> map;

  RepoReplaceResult result = ReplaceFetchContentRepos(content, map);

  ASSERT_EQ(result.missing.size(), 1u);
  EXPECT_EQ(result.missing[0], "https://example.com/missing.git");
  EXPECT_EQ(result.output, content);
}
