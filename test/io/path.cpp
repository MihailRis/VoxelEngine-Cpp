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

TEST(Path, DotElements) {
    io::path p("entry_point:a/b/c/../../d/e/../");
    EXPECT_EQ(p.normalized(), "entry_point:a/d");
    EXPECT_EQ(io::path("test:a///b//////c/").parent(), io::path("test:a///b"));
}
