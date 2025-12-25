#include "allowlist.h"

#include <gtest/gtest.h>

TEST(Allowlist, ParsesLibraryAllowlist) {
  std::string content =
      "# comment\n"
      "-lssl\n"
      "crypto\n"
      "\"z\"\n";

  auto allowlist = ParseAllowlist(content);

  EXPECT_EQ(allowlist.count("ssl"), 1u);
  EXPECT_EQ(allowlist.count("crypto"), 1u);
  EXPECT_EQ(allowlist.count("z"), 1u);
}

TEST(Allowlist, ParsesRepoMap) {
  std::string content =
      "https://example.com/a.git => https://mirror.local/a.git\n"
      "\"https://example.com/b.git\" => \"https://mirror.local/b.git\"\n";

  auto repo_map = ParseRepoMap(content);

  ASSERT_EQ(repo_map.size(), 2u);
  EXPECT_EQ(repo_map["https://example.com/a.git"], "https://mirror.local/a.git");
  EXPECT_EQ(repo_map["https://example.com/b.git"], "https://mirror.local/b.git");
}
