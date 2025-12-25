#include "makefile_parser.h"

#include <gtest/gtest.h>

TEST(MakefileParser, ExtractsLibrariesInOrder) {
  std::string content =
      "LIBS = -lssl -lcrypto\n"
      "# -lz should be ignored\n"
      "LDFLAGS = -Wl,-lfoo -l:libbar.a\n";

  std::vector<std::string> libs = ExtractLibrariesFromMakefile(content);

  ASSERT_EQ(libs.size(), 4u);
  EXPECT_EQ(libs[0], "ssl");
  EXPECT_EQ(libs[1], "crypto");
  EXPECT_EQ(libs[2], "foo");
  EXPECT_EQ(libs[3], "libbar.a");
}
