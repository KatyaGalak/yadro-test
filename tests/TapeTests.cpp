#include <gtest/gtest.h>
#include "TapeConfig.hpp"
#include "FileTape.hpp"
#include <filesystem>

TEST(ConfigTest, DefaultValues) {
    TapeConfig cfg;
    EXPECT_EQ(cfg.delayReadMs, 0);
}

TEST(TapeTest, Creation) {
    TapeConfig cfg;

    EXPECT_NO_THROW({
        FileTape tape("test_empty.bin", cfg);
    });
    std::filesystem::remove("test_empty.bin");
}