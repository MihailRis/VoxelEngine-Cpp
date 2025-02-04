#include <gtest/gtest.h>

#include "io/path.hpp"

TEST(Path, Path) {
    io::path p("entry_point:path/file.ext");
    EXPECT_EQ(p, "entry_point:path/file.ext");
    EXPECT_EQ(p.pathPart(), "path/file.ext");
    EXPECT_EQ(p.name(), "file.ext");
    EXPECT_EQ(p.extension(), ".ext");
    EXPECT_EQ(p.entryPoint(), "entry_point");
    EXPECT_EQ(p / "child", "entry_point:path/file.ext/child");
    EXPECT_EQ(p.parent(), "entry_point:path");
}
